

/*

代码考核：

给定字符串 s，只包含字符 '(' ')' '{' '}' '[' ']'，判断括号是否匹配且顺序正确。
规则：左括号必须被相同类型的右括号闭合，且闭合顺序必须正确。

样例

s="()[]{}" → true

s="([{}])" → true

s="(]" → false

s="([)]" → false

s="((()))" → true

s="{)}" → false

*/

#include <iostream>
#include <string>

int main()
{
    std::string s = "";

    if (s.size() % 2 != 0)
    {
        std::cout << "false" << std::endl;
    }
    else
    {
        int head = 0;
        int tail = s.size() - 1;

        bool is_success = false;

        for (size_t index = 0; index < s.size(); idnex++)
        {
            if ((s[index] == "(" && s[index + 1] == ")") || s[index] == "[" && s[index + 1] == "]" || s[index] == "{" && s[index + 1] == "}")
            {
                index = index + 2;
            }
            else
            {
                break;
            }
            if (index >= s.size())
            {
                std::cout << "true" << std::endl;
                is_success = true;
                break;
            }
        }

        if (!is_success)
        {
            while (true)
            {
                if (head + 1 == tail)
                {
                    std::cout << "true" << std::endl;
                    break;
                }

                if ((s[head] == "(" && s[tail] == ")") || s[head] == "[" && s[tail] == "]" || s[head] == "{" && s[tail] == "}")
                {
                    head = head + 1;
                    tail = tail - 1;
                }
                else
                {
                    std::cout << "false" << std::endl;
                    break;
                }
            }
        }
    }
}