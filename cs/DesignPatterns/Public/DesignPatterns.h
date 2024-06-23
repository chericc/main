#pragma once

#define DP_BEGIN namespace DESIGNPATTERNS {
#define DP_END }

#define PRIVATE_BEGIN namespace {
#define PRIVATE_END }

#define EXAMPLE_BEGIN namespace EXAMPLE {
#define EXAMPLE_END }

DP_BEGIN

class DesignPattern {
   public:
    virtual void Play() = 0;
};

DP_END