#pragma once
#include "DesignPatterns.h"

DP_BEGIN

class DPAdapter :
	public DesignPattern
{
public:
	void Play() override;
};

DP_END