#include "common.hpp"
#include <cassert>
#include <concepts>
#include <span>
#include <string>
#include <vector>


namespace NF::Serialization
{
    class BinaryWriter
    {
    public:
        BinaryWriter(std::vector<unsigned char>&) {}
    };
};


namespace NF::EditorLink
{
    // public api
    enum class MessageType : u16
    {
        Hello,
        Quit,
        Log,
        WindowHandle,
    };
    
    namespace Internal
    {
        void SendMessageInternal(
            [[maybe_unused]] MessageType type,
            [[maybe_unused]] std::span<unsigned char> span
        )
        {
        }
    }
    struct HelloMessage
    {
        static constexpr MessageType MSG_TYPE{ MessageType::Hello };
        
        u16 protocolVersion{};
        u16 engineMajor{};
        u16 engineMinor{};
        u16 enginePatch{};
        void Send() const;
    };
    
    struct QuitMessage
    {
        static constexpr MessageType MSG_TYPE{ MessageType::Quit };
        void Send() const;
    };
    

    template <typename T>
    concept MsgStruct = requires (NF::Serialization::BinaryWriter& writer, const T& m)
    {
        { T::MSG_TYPE };
        { m.Send() };
        
    };
    
    void SendMessage(const MsgStruct auto& msg)
    {
        std::vector<unsigned char> vec;
        Serialization::BinaryWriter writer(vec);
        msg.Encode(writer);
        Internal::SendMessageInternal(msg.MSG_TYPE, vec);
    }
};


// in practise
int main()
{
    NF::EditorLink::HelloMessage h{1, 1, 1, 1};
    NF::EditorLink::QuitMessage q{};
    NF::EditorLink::LogMessage l{1, "LogMessage"};
    NF::EditorLink::InvalidMessage i{1};

    h.Send()
    
    NF::EditorLink::SendMessage(h);
    NF::EditorLink::SendMessage(q);
    NF::EditorLink::SendMessage(l);
    // QueueUp(i); this fails here
    
    return 0;
}
