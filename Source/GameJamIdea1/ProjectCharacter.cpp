// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectCharacter.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AttributeSetBase.h"
#include "GameJamIdea1GameMode.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProjectCharacter::AProjectCharacter() {
  // Set this character to call Tick() every frame.  You can turn this off to
  // improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  SystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(FName("Ability System Component"));
  AttributeSetBase = CreateDefaultSubobject<UAttributeSetBase>(FName("Attribute Set Base Component"));
}

// Called when the game starts or when spawned
void AProjectCharacter::BeginPlay() {
  Super::BeginPlay();

  AGameModeBase* CurrentGameMode = UGameplayStatics::GetGameMode(this);
  if (CurrentGameMode) {
    AGameJamIdea1GameMode* GM = Cast<AGameJamIdea1GameMode>(CurrentGameMode);
    if (GM) {
      GameMode = GM;
    }
  }

  if (ensure(AttributeSetBase)) {
    AttributeSetBase->OnHealthChanged.AddDynamic(this, &AProjectCharacter::OnHealthChanged);
    AttributeSetBase->OnManaChanged.AddDynamic(this, &AProjectCharacter::OnManaChanged);
    AttributeSetBase->OnMainAttributeChanged.AddDynamic(this, &AProjectCharacter::OnMainAttributeChanged);
  }
  bIsDead = false;
}

// Called every frame
void AProjectCharacter::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void AProjectCharacter::AcquireAbility(TSubclassOf<UGameplayAbility> AbilityToAcquire) {
  if (!ensure(SystemComp))
    return;
  if (!HasAuthority())
    return;
  if (AbilityToAcquire) {
    FGameplayAbilitySpecDef SpecDef = FGameplayAbilitySpecDef();
    SpecDef.Ability = AbilityToAcquire;
    FGameplayAbilitySpec Spec = FGameplayAbilitySpec(SpecDef, 1);
    SystemComp->GiveAbility(Spec);
  }
  SystemComp->InitAbilityActorInfo(this, this);
}

void AProjectCharacter::AcquireAbilities(TArray<TSubclassOf<UGameplayAbility>> AbilitiesToAcquire) {
  if (HasAuthority()) {
    for (TSubclassOf<UGameplayAbility> AbilityClass : AbilitiesToAcquire) {
      AcquireAbility(AbilityClass);
    }
  }
}

// Called to bind functionality to input
void AProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
  PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);

  PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
  PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

  PlayerInputComponent->BindAxis("MoveForward", this, &AProjectCharacter::MoveForward);
  PlayerInputComponent->BindAxis("MoveRight", this, &AProjectCharacter::MoveRight);
}

bool AProjectCharacter::IsOtherActorHostile(AActor* OtherActor) 
{
  return IsOtherHostile(OtherActor);
}

void AProjectCharacter::PossessedBy(AController* NewController) 
{
  Super::PossessedBy(NewController);
  SetTeamIDByControllerType();
}

void AProjectCharacter::MoveForward(float Value) {
  if (Value == 0 || !ensure(GetController()))
    return;
  FRotator YawRotation(0, GetController()->GetControlRotation().Yaw, 0);

  FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

  AddMovementInput(ForwardVector, Value);
}

void AProjectCharacter::MoveRight(float Value) {
  if (Value == 0 || !ensure(GetController()))
    return;
  FRotator YawRotation(0, GetController()->GetControlRotation().Yaw, 0);

  FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

  AddMovementInput(RightVector, Value);
}

void AProjectCharacter::OnHealthChanged(float Health, float MaxHealth, bool FullHealth, float PreviousHealth, AActor* EffectInstigator) {
  if (bIsDead)
    return;
  BP_OnHealthChanged(Health, MaxHealth, FullHealth, PreviousHealth, EffectInstigator);
	if (Health <= 0.f) {
		bIsDead = true;
		BP_OnDeath();
	}
}

void AProjectCharacter::OnManaChanged(float Mana, float MaxMana) 
{
  BP_OnManaChanged(Mana, MaxMana);
}

void AProjectCharacter::OnMainAttributeChanged(EAttributeType Type, float CurrentAttributeValue) 
{
  BP_OnMainAttributeChanged(Type, CurrentAttributeValue);
}

void AProjectCharacter::SetTeamIDByControllerType() 
{
  if (!GetController()) return;
	if (IsPlayerControlled()) {
		TeamID = 0;
	}
	else {
		TeamID = 1;
	}
}

