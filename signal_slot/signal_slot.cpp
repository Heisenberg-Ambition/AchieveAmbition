
#include <functional>
#include <iostream>

class Button
{
public:
    std::function<void()> onClick;

    void click()
    {
        if (onClick)
        {
            onClick();
        }
    }

    Button()
    {
        std::cout << "Button constructor\n";
    }
};

int main()
{
    Button btn;

    btn.onClick = []() { std::cout << "clicked\n"; };

    btn.click();
}