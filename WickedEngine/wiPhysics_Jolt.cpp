#if 1
#include "wiPhysics.h"

#include <Jolt/Jolt.h>


namespace wi::physics
{
	namespace jolt
	{

	}

	using namespace JPH;
	using namespace JPH::literals;

	void Initialize() {}

	void SetEnabled(bool value) {}
	bool IsEnabled() { return true; }

	void SetSimulationEnabled(bool value) {}
	bool IsSimulationEnabled() { return true; }

	void SetDebugDrawEnabled(bool value) {}
	bool IsDebugDrawEnabled() { return false; }

	void SetAccuracy(int value) {}
	int GetAccuracy() { return 42; }

	void RunPhysicsUpdateSystem(
		wi::jobsystem::context& ctx,
		wi::scene::Scene& scene,
		float dt
	) {}

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
