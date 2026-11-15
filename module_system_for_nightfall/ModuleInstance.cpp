#include "ModuleInstance.hpp"

Mod::Mod()
    : something(new int(3))
{
    
}

Mod::~Mod()
{
    delete something;
}

void Mod::BeginFrame()  noexcept
{
    
}
void Mod::PreUpdate()   noexcept
{
    
}
void Mod::FixedUpdate() noexcept
{
    
}
void Mod::Update()      noexcept
{
    
}
void Mod::PostUpdate()  noexcept
{
    
}
void Mod::Render()      noexcept
{
    
}
void Mod::EndFrame()    noexcept
{
    
}

auto Mod::GetSomething() const noexcept
{
    return something;
}
