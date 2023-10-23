#if 1
#include "wiPhysics.h"
#include "wiBacklog.h"
#include "wiScene.h"
#include "wiTimer.h"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/PhysicsScene.h>


using namespace wi::scene;
using namespace wi::ecs;

using namespace JPH;
//using namespace JPH::literals;

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
		std::unique_ptr<PhysicsSystem> physics_system;
		std::unique_ptr<JobSystemWithBarrier> job_system;


		PhysicsScene& GetPhysicsScene(Scene& scene)
		{
			if (scene.physics_scene == nullptr)
			{
				auto pscene = std::make_shared<PhysicsScene>();



				scene.physics_scene = pscene;
			}
			return *(PhysicsScene*)scene.physics_scene.get();
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
		physics_system = std::make_unique<PhysicsSystem>();

		physics_system->Init(MAX_BODIES, NUM_BODY_MUTEXES, MAX_BODY_PAIRS, MAX_CONTACT_CONSTRAINTS, broad_phase_layer_interface, object_vs_broad_phase_layer_filter, object_layer_pair_filter);

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

	void ApplyImpulseAt(
		wi::scene::RigidBodyPhysicsComponent& physicscomponent,
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

}
#endif
