#ifndef TYPE_REGISTRY_H
#define TYPE_REGISTRY_H
#include <cstring>
#include <unordered_map>
class type_registry{
    typedef std::unordered_map<int, std::string> registry;
public:
    inline static registry types = registry();

    static std::string get_typename(int id) {
        if(types.find(id) == types.end()) {
            return "invalid_type";
        }
        return types[id];
    }

    static int reverse_lookup(const std::string& name) {
        for(auto& pair : types) {
            if(pair.second == name) {
                return pair.first;
            }
        }
        return -1;
    }

    static void add_type(int id, const std::string& name) {
        types.insert(std::make_pair(id,name));
    }
};
#endif //TYPE_REGISTRY_H
