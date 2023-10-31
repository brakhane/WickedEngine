#include "wiBacklog.h"
#include "wiPhysics.h"
#include "wiScene.h"
#include "wiTimer.h"

#include <Jolt/Jolt.h>

#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Geometry/Triangle.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>


using namespace wi::scene;
using namespace wi::ecs;

using namespace JPH;
//using namespace JPH::literals;

template<typename T, typename F> T to(const F f) { return static_cast<T>(f); }

template<>
Vec3 to(const XMFLOAT3 f)
{
	return Vec3(f.x, f.y, f.z);
}

template<>
Float3 to(const XMFLOAT3 f)
{
	return Float3(f.x, f.y, f.z);
}

template<>
Quat to(const XMFLOAT4 f)
{
	return Quat(f.x, f.y, f.z, f.w);
}

namespace wi::physics
{
	namespace jolt
	{
		bool enabled = true;
		bool simulation_enabled = true;
		bool debugdraw_enabled = false;

		// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
		const uint MAX_BODIES = 65536;
		// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
		const uint NUM_BODY_MUTEXES = 0;
		// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
		// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
		// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
		const uint MAX_BODY_PAIRS = 65536;
		// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
		// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
		const uint MAX_CONTACT_CONSTRAINTS = 10240;

		// simple default implementations for now (always return true)
		class DummyBPLI final : public BroadPhaseLayerInterface {
			BroadPhaseLayer dummy;
			virtual uint GetNumBroadPhaseLayers() const
			{
				return 1;
			}
			virtual BroadPhaseLayer	GetBroadPhaseLayer(ObjectLayer inLayer) const
			{
				return dummy;
			}
			virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const
			{
				return "dummy";
			}
		};

		DummyBPLI broad_phase_layer_interface;
		ObjectVsBroadPhaseLayerFilter object_vs_broad_phase_layer_filter;
		ObjectLayerPairFilter object_layer_pair_filter;

		std::unique_ptr<TempAllocator> temp_allocator;
		std::unique_ptr<JobSystemWithBarrier> job_system;
		ObjectLayer dummy_layer;
		std::shared_ptr<PhysicsSystem> TEST;

		PhysicsSystem& GetOrCreatePhysicsSystem(Scene& scene)
		{
			if (scene.physics_scene == nullptr)
			{
				if (TEST == nullptr)
				{
					TEST = std::make_shared<PhysicsSystem>();

					TEST->Init(MAX_BODIES, NUM_BODY_MUTEXES, MAX_BODY_PAIRS, MAX_CONTACT_CONSTRAINTS, broad_phase_layer_interface, object_vs_broad_phase_layer_filter, object_layer_pair_filter);
				}
				scene.physics_scene = TEST;
			}
			return *(PhysicsSystem*)scene.physics_scene.get();
		}
		void AddRigidBody(Scene& scene, Entity entity, RigidBodyPhysicsComponent& physics_component, const TransformComponent& transform, const MeshComponent* mesh)
		{
			PhysicsSystem& physics_system = GetOrCreatePhysicsSystem(scene);
			BodyInterface& body_interface = physics_system.GetBodyInterface();
			auto pos = transform.GetPosition();
			auto rot = transform.GetRotation();
			Vec3 scale = to<Vec3>(transform.GetScale());

			Ref<Shape> shape;

			switch (physics_component.shape)
			{
			case RigidBodyPhysicsComponent::CollisionShape::BOX:
				shape = new BoxShape(Vec3(physics_component.box.halfextents.x, physics_component.box.halfextents.y, physics_component.box.halfextents.z) * scale);
				//shape = new ScaledShape(shape, scale);
				break;
			case RigidBodyPhysicsComponent::CollisionShape::SPHERE:
				shape = new SphereShape(physics_component.sphere.radius);
				break;
			case RigidBodyPhysicsComponent::CollisionShape::CAPSULE:
				shape = new CapsuleShape(physics_component.capsule.height, physics_component.capsule.radius);
				break;
			case RigidBodyPhysicsComponent::CollisionShape::CONVEX_HULL:
				if (mesh != nullptr)
				{
					Array<Vec3> points;
					points.reserve(mesh->vertex_positions.size());
					for (auto& pos : mesh->vertex_positions)
					{
						points.emplace_back(to<Vec3>(pos) * scale);
					}
					shape = ConvexHullShapeSettings(points).Create().Get();
					//shape = new ScaledShape(shape, scale);
				}
				else
				{
					wi::backlog::post("Convex Hull physics requested, but no MeshComponent provided!");
					assert(0);
				}
				break;
			case RigidBodyPhysicsComponent::CollisionShape::TRIANGLE_MESH:
				if (mesh != nullptr)
				{
					int totalTriangles = 0;
					int* indices = nullptr;
					uint32_t first_subset = 0;
					uint32_t last_subset = 0;
					mesh->GetLODSubsetRange(physics_component.mesh_lod, first_subset, last_subset);
					for (uint32_t subsetIndex = first_subset; subsetIndex < last_subset; ++subsetIndex)
					{
						const MeshComponent::MeshSubset& subset = mesh->subsets[subsetIndex];
						if (indices == nullptr)
						{
							indices = (int*)(mesh->indices.data() + subset.indexOffset);
						}
						totalTriangles += int(subset.indexCount / 3);
					}
					Array<Triangle> triangles;
					triangles.reserve(totalTriangles);
					for (auto it = mesh->vertex_positions.begin(); it != mesh->vertex_positions.end(); )
					{
						auto v1 = *it++;
						auto v2 = *it++;
						auto v3 = *it++;

						triangles.emplace_back(Triangle(to<Float3>(v1), to<Float3>(v2), to<Float3>(v3)));
					}

					shape = MeshShapeSettings(triangles).Create().Get();
				}
				else
				{
					wi::backlog::post("Triangle Mesh physics requested, but no MeshComponent provided!");
					assert(0);
				}
			}
			if (shape != nullptr)
			{
				BodyCreationSettings body_settings(shape, to<Vec3>(pos), to<Quat>(rot), entity == 6 ? EMotionType::Static : EMotionType::Dynamic, dummy_layer);
				body_settings.mUserData = entity;
				BodyID id = body_interface.CreateAndAddBody(body_settings, EActivation::Activate);
				if (!id.IsInvalid())
				{
					physics_component.physicsobject = std::make_shared<BodyID>(id);
				}
				//delete shape;
			}

		}

	}
	using namespace jolt;


	void Initialize()
	{
		wi::Timer timer;

		RegisterDefaultAllocator();
		Factory::sInstance = new Factory();
		RegisterTypes();

		job_system = std::make_unique<JobSystemThreadPool>(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);
		temp_allocator = std::make_unique<TempAllocatorImpl>(10 * 1024 * 1024);


		wi::backlog::post("wi::physics Initialized [Jolt] (" + std::to_string((int)std::round(timer.elapsed())) + " ms)");
	}

	void SetEnabled(bool value) { enabled = value; }
	bool IsEnabled() { return enabled; }

	void SetSimulationEnabled(bool value) { simulation_enabled = value; }
	bool IsSimulationEnabled() { return simulation_enabled; }

	void SetDebugDrawEnabled(bool value) { debugdraw_enabled = value; }
	bool IsDebugDrawEnabled() { return debugdraw_enabled; }

	// TODO
	void SetAccuracy(int value) {}
	int GetAccuracy() { return 42; }

	void RunPhysicsUpdateSystem(
		wi::jobsystem::context& ctx,
		wi::scene::Scene& scene,
		float dt
	)
	{
		PhysicsSystem& psys = GetOrCreatePhysicsSystem(scene);

		psys.SetGravity(Vec3(scene.weather.gravity.x, scene.weather.gravity.y, scene.weather.gravity.z));

		for (int i = 0; i < scene.rigidbodies.GetCount(); i++)
		{
			RigidBodyPhysicsComponent& physics_component = scene.rigidbodies[i];
			Entity entity = scene.rigidbodies.GetEntity(i);

			if (physics_component.physicsobject == nullptr && scene.transforms.Contains(entity))
			{
				TransformComponent& transform = *scene.transforms.GetComponent(entity);
				const ObjectComponent* object = scene.objects.GetComponent(entity);
				const MeshComponent* mesh = nullptr;
				if (object != nullptr)
				{
					mesh = scene.meshes.GetComponent(object->meshID);
				}
				AddRigidBody(scene, entity, physics_component, transform, mesh);
				//scene.Component_Detach(entity); // needed???

			}
		}

		if (IsSimulationEnabled())
		{
			psys.Update(dt, 1, &*temp_allocator, &*job_system);
		}


		BodyInterface& body_interface = psys.GetBodyInterface();

		BodyIDVector body_ids;

		psys.GetBodies(body_ids);

		const BodyLockInterface& bli = psys.GetBodyLockInterface();

		for (const BodyID& id : body_ids)
		{
			BodyLockRead lock(bli, id);
			if (lock.Succeeded())
			{
				const Body& body = lock.GetBody();
				TransformComponent* transform = scene.transforms.GetComponent(body.GetUserData());

				if (transform != nullptr)
				{
					auto pos = body.GetPosition();
					auto rot = body.GetRotation();

					transform->translation_local = XMFLOAT3(pos.GetX(), pos.GetY(), pos.GetZ());
					transform->rotation_local = XMFLOAT4(rot.GetX(), rot.GetY(), rot.GetZ(), rot.GetW());
					transform->SetDirty();
				}
			}
		}
	}

	void SetLinearVelocity(
		wi::scene::RigidBodyPhysicsComponent& physicscomponent,
		const XMFLOAT3& velocity
	) {}

	void SetAngularVelocity(
		wi::scene::RigidBodyPhysicsComponent& physicscomponent,
		const XMFLOAT3& velocity
	) {}

	void ApplyForce(
		wi::scene::RigidBodyPhysicsComponent& physicscomponent,
		const XMFLOAT3& force
	) {}

	void ApplyForceAt(
		wi::scene::RigidBodyPhysicsComponent& physicscomponent,
		const XMFLOAT3& force,
		const XMFLOAT3& at
	) {}

	void ApplyImpulse(
		wi::scene::RigidBodyPhysicsComponent& physicscomponent,
		const XMFLOAT3& impulse
	) {}

	void ApplyImpulse(
		wi::scene::HumanoidComponent& humanoid,
		wi::scene::HumanoidComponent::HumanoidBone bone,
		const XMFLOAT3& impulse
	) {}
	void ApplyImpulseAt(
		wi::scene::RigidBodyPhysicsComponent& physicscomponent,
		const XMFLOAT3& impulse,
		const XMFLOAT3& at
	) {}
	void ApplyImpulseAt(
		wi::scene::HumanoidComponent& humanoid,
		wi::scene::HumanoidComponent::HumanoidBone bone,
		const XMFLOAT3& impulse,
		const XMFLOAT3& at
	) {}
	void ApplyTorque(
		wi::scene::RigidBodyPhysicsComponent& physicscomponent,
		const XMFLOAT3& torque
	) {}
	void ApplyTorqueImpulse(
		wi::scene::RigidBodyPhysicsComponent& physicscomponent,
		const XMFLOAT3& torque
	) {}

	void SetActivationState(
		wi::scene::RigidBodyPhysicsComponent& physicscomponent,
		ActivationState state
	) {}
	void SetActivationState(
		wi::scene::SoftBodyPhysicsComponent& physicscomponent,
		ActivationState state
	) {}
	RayIntersectionResult Intersects(
		const wi::scene::Scene& scene,
		wi::primitive::Ray ray
	) {return RayIntersectionResult();}
	void PickDrag(
		const wi::scene::Scene& scene,
		wi::primitive::Ray ray,
		PickDragOperation& op
	) {}

}
