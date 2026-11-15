class IModule
{
private:
    
    // friend class Application
    void BeginFrame()  noexcept {}
    void PreUpdate()   noexcept {}
    void FixedUpdate() noexcept {}
    void Update()      noexcept {}
    void PostUpdate()  noexcept {}
    void Render()      noexcept {}
    void EndFrame()    noexcept {}

public:
    
    /* --- public API --- */
    //...
};
