// Copyright Epic Games, Inc. All Rights Reserved.

#include "homework8Character.h"
#include "SpartaGameStateBase.h"
#include "Engine/LocalPlayer.h"
#include "SpartaGameInstance.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// Ahomework8Character

Ahomework8Character::Ahomework8Character()
	:HUDWidgetClass(nullptr),
	HUDWidgetInstance(nullptr),
	MainMenuWidgetClass(nullptr),
	MainMenuWidgetInstance(nullptr)

{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	Health = MaxHealth;
	MaxHealth = 100.0f;


	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void Ahomework8Character::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void Ahomework8Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &Ahomework8Character::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &Ahomework8Character::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void Ahomework8Character::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void Ahomework8Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

float Ahomework8Character::GetHealth() const
{
	return Health;
}

void Ahomework8Character::AddHealth(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	UE_LOG(LogTemplateCharacter, Warning, TEXT("Health Increased to : %f"), Health);
}

float Ahomework8Character::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);
	UE_LOG(LogTemplateCharacter, Warning, TEXT("Health: %f"), Health);
	if (Health <= 0.0f)
	{
		// Die!
		OnDeath();
	}
	return ActualDamage;
}

void Ahomework8Character::OnDeath()
{
}

void Ahomework8Character::BeginPlay()
{
	Super::BeginPlay();
	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
		}
		
	}
	ASpartaGameStateBase* SpartaGameState = GetWorld()? GetWorld()->GetGameState<ASpartaGameStateBase>() :nullptr;
	if (SpartaGameState)
	{
		SpartaGameState->UpdateHUD();
	}
}

UUserWidget* Ahomework8Character::GetHUDWidget() const
{
	return HUDWidgetInstance;
}

void Ahomework8Character::ShowMainMenu(bool bIsRestart)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();

			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				PC->bShowMouseCursor = true; //  마우스 커서 활성화
				PC->SetInputMode(FInputModeGameAndUI()); //  UI와 게임 입력을 모두 받을 수 있도록 설정
			}

			if (UTextBlock* ButtonText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("StartButtonText"))))
			{
				if (bIsRestart)
				{
					ButtonText->SetText(FText::FromString(TEXT("Restart")));
				}
				else
				{
					ButtonText->SetText(FText::FromString(TEXT("start")));
				}
			}

		}
	}
}

void Ahomework8Character::ShowGameHUD()
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}
	if (!HUDWidgetClass) //  nullptr 체크 추가
	{
		UE_LOG(LogTemp, Error, TEXT("ShowGameHUD() 실패: HUDWidgetClass가 nullptr! 블루프린트에서 설정 필요."));
		return;
	}
	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();

			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				PC->bShowMouseCursor = false; 
				PC->SetInputMode(FInputModeGameOnly()); 
			}

			ASpartaGameStateBase* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameStateBase>() : nullptr;
			if (SpartaGameState)
			{
				SpartaGameState->UpdateHUD();
			}
		}
	}
}

void Ahomework8Character::StartGame()
{
	if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		SpartaGameInstance->CurrentLevelIndex = 0;
		SpartaGameInstance->TotalScore = 0;

	}

	UGameplayStatics::OpenLevel(GetWorld(), FName("BasicLevel"));
}