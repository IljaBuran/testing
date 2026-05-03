#include <iostream>

class Animal
{
public:

    Animal()
    {
        std::cout << "Animal constructor\n";
    }

    virtual void make_noise() const
    {
        std::cout << "No Noise\n";
    }
};

class Dog : Animal
{
public:

    Dog()
    {
        std::cout << "Dog constructor\n";
    }

    void make_noise() const override
    {
        std::cout << "Wuf\n";
    }
};

int main()
{
    Animal animal;
    animal.make_noise();

    Dog dog;
    dog.make_noise();

    Animal* pA = (Animal*)(&dog);
    pA->make_noise();
    Dog* pD = (Dog*)(&animal);
    pD->make_noise();


    return 0;
}
