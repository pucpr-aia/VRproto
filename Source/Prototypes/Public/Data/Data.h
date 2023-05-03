#pragma once

#include "Data.generated.h"

UENUM()
enum class EGrabType : uint8
{
	None,
	Free,
	Snap,
	Custom
};