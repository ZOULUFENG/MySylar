#include <yaml-cpp/yaml.h>

#include "../sylar/config.h"
#include "../sylar/log.h"
sylar::ConfigVar<int>::ptr g_int_value_config
    = sylar::Config::Lookup("system.port", (int)8080, "system port");

sylar::ConfigVar<float>::ptr g_float_value_config
    = sylar::Config::Lookup("system.value", (float)10.2f, "system value");

sylar::ConfigVar<std::vector<int>>::ptr g_int_vector_config
    = sylar::Config::Lookup("system.int_vec", std::vector<int> { 1, 2 }, "system int vec");

sylar::ConfigVar<std::list<int>>::ptr g_int_list_config
    = sylar::Config::Lookup("system.int_list", std::list<int> { 1, 2 }, "system int list");

sylar::ConfigVar<std::set<int>>::ptr g_int_set_config
    = sylar::Config::Lookup("system.int_set", std::set<int> { 1, 2 }, "system int set");

sylar::ConfigVar<std::unordered_set<int>>::ptr g_int_unordered_set_config
    = sylar::Config::Lookup("system.int_uset", std::unordered_set<int> { 1, 2 }, "system int unordered_set");

// init list mode to init map
sylar::ConfigVar<std::map<std::string, int>>::ptr g_int_map_value_config
    = sylar::Config::Lookup("system.str_int_map", std::map<std::string, int> { { "k", 2 } }, "system str int map");

sylar::ConfigVar<std::unordered_map<std::string, int>>::ptr g_int_umap_value_config
    = sylar::Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int> { { "k", 2 } }, "system str int umap");

void print_yaml(const YAML::Node& node, int level);

void test_yaml()
{
    YAML::Node root = YAML::LoadFile("/home/gl/workspace/sylar/bin/conf/log.yml");
    print_yaml(root, 0);
    // SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << root;
}

void test_config()
{
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
        << "before: " << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
        << "before: " << g_float_value_config->toString();
#define XX(g_var, name, prefix)                                                               \
    {                                                                                         \
        auto& v = g_var->getValue();                                                          \
        for (auto& i : v) {                                                                   \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name ": " << i;                  \
        }                                                                                     \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

#define XX_M(g_var, name, prefix)                                                                               \
    {                                                                                                           \
        auto& v = g_var->getValue();                                                                            \
        for (auto& i : v) {                                                                                     \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name ": {" << i.first << " - " << i.second << "}"; \
        }                                                                                                       \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString();                   \
    }

    XX(g_int_vector_config, int_vec, before);
    XX(g_int_list_config, int_list, before);
    XX(g_int_set_config, int_set, before);
    XX(g_int_unordered_set_config, int_uset, before);
    XX_M(g_int_map_value_config, str_int_map, before);
    XX_M(g_int_umap_value_config, str_int_umap, before);

    YAML::Node root = YAML::LoadFile("/home/gl/workspace/sylar/bin/conf/log.yml");
    sylar::Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_float_value_config->toString();

    XX(g_int_vector_config, int_vec, after);
    XX(g_int_list_config, int_list, after);
    XX(g_int_set_config, int_set, after);
    XX(g_int_unordered_set_config, int_uset, after);
    XX_M(g_int_map_value_config, str_int_map, after);
    XX_M(g_int_umap_value_config, str_int_umap, after);
}

void print_yaml(const YAML::Node& node, int level)
{
    if (node.IsScalar()) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
            << std::string(level * 4, ' ') << node.Scalar() << "-IsScalar-"
            << node.Type() << "-" << level;
    } else if (node.IsNull()) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ') << "NULL - "
                                         << node.Type() << "-" << level;
    } else if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); it++) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
                << std::string(level * 4, ' ') << it->first << "-IsMap-"
                << it->second.Type() << "-" << level;
            print_yaml(it->second, level + 1);
        }
    } else if (node.IsSequence()) {
        for (size_t i = 0; i < node.size(); i++) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
                << std::string(level * 4, ' ') << i << "-" << node[i].Type()
                << "-IsSequence-" << level;
            print_yaml(node[i], level + 1);
        }
    }
}

int main(int argc, char** argv)
{
    // SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_int_value_config->getValue();
    // SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_float_value_config->toString();
    // test_yaml();
    test_config();
    return 0;
}
