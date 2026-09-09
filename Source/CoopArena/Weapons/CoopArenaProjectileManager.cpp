#include "CoopArenaProjectileManager.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CoopArenaProjectileMovement.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/CollisionProfile.h"

ACoopArenaProjectileManager::ACoopArenaProjectileManager() {
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	BallsIsm = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RestingBalls"));
	BallsIsm->SetupAttachment(RootComponent);
	BallsIsm->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	BallsIsm->SetGenerateOverlapEvents(false);
	BallsIsm->SetCanEverAffectNavigation(false);
}

void ACoopArenaProjectileManager::BeginPlay() {
	Super::BeginPlay();

	if (const auto* BallCdo = BallComponentClass.GetDefaultObject()) {
		BallsIsm->SetStaticMesh(BallCdo->GetStaticMesh());
		BallsIsm->SetMaterial(0, BallCdo->GetMaterial(0));
	}
}

void ACoopArenaProjectileManager::Launch(
	const FVector& Location, const FVector& Velocity, const FGameplayEffectSpecHandle& DamageSpec,
	AActor* ActorToIgnore
) {
	int32 SlotIdx;
	if (!BallSlotFreeIndices.IsEmpty()) {
		auto It = BallSlotFreeIndices.CreateIterator();
		SlotIdx = *It;
		It.RemoveCurrent();
	}
	else {
		const auto Mesh = NewObject<UStaticMeshComponent>(this, BallComponentClass);
		Mesh->SetVisibility(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->RegisterComponent();
		Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

		const auto Pmc = NewObject<UCoopArenaProjectileMovement>(this, MovementComponentClass);
		Pmc->SetAutoActivate(false);
		Pmc->bAutoRegisterUpdatedComponent = false;
		Pmc->RegisterComponent();
		Pmc->BindToSlot(this, BallSlots.Num());

		SlotIdx = BallSlots.Emplace(Mesh, Pmc);
	}

	{
		auto& Slot = BallSlots[SlotIdx];

		Slot.DamageSpec = DamageSpec;

		Slot.Mesh->SetWorldLocation(Location);
		Slot.Mesh->SetVisibility(true);
		Slot.Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Slot.Mesh->ClearMoveIgnoreActors();
		if (ActorToIgnore)
			Slot.Mesh->MoveIgnoreActors.Push(ActorToIgnore);

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

	if (const auto TargetAbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Hit.GetActor()))
		TargetAbilitySystem->ApplyGameplayEffectSpecToSelf(*Slot.DamageSpec.Data.Get());

	Slot.DamageSpec.Clear();
}

void ACoopArenaProjectileManager::OnBallStopped(const int32 SlotIndex) {
	if (!BallSlots.IsValidIndex(SlotIndex))
		return;

	bool AlreadyInSet = false;
	BallSlotFreeIndices.Add(SlotIndex, &AlreadyInSet);
	if (AlreadyInSet)
		return;

	auto& Slot = BallSlots[SlotIndex];

	{
		const auto& Transform = Slot.Mesh->GetComponentTransform();
		if (BallsIsm->GetInstanceCount() < BallsLimit) {
			BallsIsm->AddInstance(Transform, true);
		}
		else {
			BallsIsm->UpdateInstanceTransform(NextIsmIndex, Transform, true, true);
			NextIsmIndex = (NextIsmIndex + 1) % BallsLimit;
		}
	}

	{
		Slot.Movement->Deactivate();

		Slot.Mesh->SetVisibility(false);
		Slot.Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Slot.Mesh->ClearMoveIgnoreActors();

		Slot.DamageSpec.Clear();
	}
}
