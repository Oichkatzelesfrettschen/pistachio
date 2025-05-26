#include <cstdint>
#include <iostream>
#include <sstream>

enum class mem_opcode : uint32_t {
    Alloc = 0,
    Free = 1
};

struct mem_request {
    mem_opcode op;
    uint64_t size;
    uint64_t addr;
};

int main(int argc, char **argv)
{
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <mr0> <mr1> <mr2> <mr3>\n";
        return 1;
    }

    uint64_t words[4];
    for (int i = 0; i < 4; ++i) {
        std::stringstream ss;
        ss << std::hex << argv[i + 1];
        ss >> words[i];
    }

    uint64_t label = words[0];
    mem_request req{
        static_cast<mem_opcode>(words[1]),
        words[2],
        words[3]
    };

    std::cout << "label: 0x" << std::hex << label << std::dec << "\n";
    const char *opstr = req.op == mem_opcode::Alloc ? "Alloc" :
                        (req.op == mem_opcode::Free ? "Free" : "Unknown");
    std::cout << "  op: " << opstr << "\n";
    std::cout << "  size: " << req.size << "\n";
    std::cout << "  addr: 0x" << std::hex << req.addr << std::dec << "\n";
    return 0;
}
