#include "IModule.hpp"

class Mod final : public IModule
{
private:

    Mod();
    ~Mod();
    
    // friend class Application
    void BeginFrame()  noexcept;
    void PreUpdate()   noexcept;
    void FixedUpdate() noexcept;
    void Update()      noexcept;
    void PostUpdate()  noexcept;
    void Render()      noexcept;
    void EndFrame()    noexcept;
    
public:
    
    auto GetSomething() const noexcept;

private:

    int *something;
};
