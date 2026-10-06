#include "CoopArenaProjectileManager.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "CoopArenaProjectileMovementComponent.h"
#include "Components/InstancedStaticMeshComponent.h"

ACoopArenaProjectileManager::ACoopArenaProjectileManager() {
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

	BallsIsm = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RestingBalls"));
	BallsIsm->PrimaryComponentTick.bCanEverTick = false;
	BallsIsm->PrimaryComponentTick.bStartWithTickEnabled = false;
	BallsIsm->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	BallsIsm->SetupAttachment(RootComponent);
	BallsIsm->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}

void ACoopArenaProjectileManager::BeginPlay() {
	Super::BeginPlay();

	if (const auto BallCdoPtr = BallComponentClass.GetDefaultObject()) {
		BallsIsm->SetStaticMesh(BallCdoPtr->GetStaticMesh());
		BallsIsm->SetMaterial(0, BallCdoPtr->GetMaterial(0));
	}
}

void ACoopArenaProjectileManager::Launch(
	const FVector& Location, const FVector& Velocity, const FGameplayEffectSpecHandle& DamageSpec,
	AActor* ActorToIgnore
) {
	int32 SlotIndex;
	if (!BallSlotFreeIndices.IsEmpty()) {
		// take a free slot from the pool
		auto It = BallSlotFreeIndices.CreateIterator();
		SlotIndex = *It;
		It.RemoveCurrent();
	}
	else {
		// no free slot: grow the pool by a new pair

		if (!BallComponentClass || !MovementComponentClass)
			return;

		const auto MeshPtr = NewObject<UStaticMeshComponent>(this, BallComponentClass);
		MeshPtr->SetVisibility(false);
		MeshPtr->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshPtr->RegisterComponent();
		MeshPtr->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

		const auto MovementPtr = NewObject<UCoopArenaProjectileMovementComponent>(this, MovementComponentClass);
		MovementPtr->Init(this, BallSlots.Num());
		MovementPtr->RegisterComponent();

		SlotIndex = BallSlots.Emplace(MeshPtr, MovementPtr);
	}

	{
		// launch (the thrower is ignored till the first hit)

		auto& Slot = BallSlots[SlotIndex];

		Slot.DamageSpec = DamageSpec;
		if (Slot.DamageSpec.IsValid())
			Slot.DamageSpec.Data->GetContext().AddOrigin(Location);

		Slot.Mesh->SetWorldLocation(Location);
		Slot.Mesh->SetVisibility(true);
		Slot.Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Slot.Mesh->IgnoreActorWhenMoving(ActorToIgnore, true);

		Slot.Movement->SetUpdatedComponent(Slot.Mesh);
		Slot.Movement->Velocity = Velocity;
		Slot.Movement->Activate(true);
	}
}

void ACoopArenaProjectileManager::OnBallImpact(const int32 SlotIndex, const FHitResult& Hit) {
	if (!BallSlots.IsValidIndex(SlotIndex) || BallSlotFreeIndices.Contains(SlotIndex))
		return;
	auto& Slot = BallSlots[SlotIndex];

	Slot.Mesh->ClearMoveIgnoreActors();
	if (!Slot.DamageSpec.IsValid())
		return;

	if (const auto TargetAbilitySystemPtr = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Hit.GetActor())) {
		Slot.DamageSpec.Data->GetContext().AddHitResult(Hit);
		TargetAbilitySystemPtr->ApplyGameplayEffectSpecToSelf(*Slot.DamageSpec.Data.Get());
	}

	Slot.DamageSpec.Clear();
}

void ACoopArenaProjectileManager::OnBallStopped(const int32 SlotIndex, const bool bCreateIsmCopy) {
	if (!BallSlots.IsValidIndex(SlotIndex))
		return;

	bool bAlreadyInSet = false;
	BallSlotFreeIndices.Add(SlotIndex, &bAlreadyInSet);
	if (bAlreadyInSet)
		return;

	auto& Slot = BallSlots[SlotIndex];

	if (bCreateIsmCopy) {
		// leave a resting ism copy
		const auto& Transform = Slot.Mesh->GetComponentTransform();
		if (BallsIsm->GetInstanceCount() < BallsLimit) {
			BallsIsm->AddInstance(Transform, true);
		}
		else {
			// past the limit it overwrites the oldest
			BallsIsm->UpdateInstanceTransform(NextIsmIndex, Transform, true, true);
			NextIsmIndex = (NextIsmIndex + 1) % BallsLimit;
		}
	}

	{
		// back to the pool

		Slot.Movement->Deactivate();

		Slot.Mesh->SetVisibility(false);
		Slot.Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Slot.Mesh->ClearMoveIgnoreActors();

		Slot.DamageSpec.Clear();
	}
}
