#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

struct ListNode 
{
    ListNode* prev = nullptr;
    ListNode* next = nullptr;
    ListNode* rand = nullptr;
    std::string data;
};

ListNode* readListFromFile(const std::string& fName)
{
    std::ifstream file(fName);
    if (!file.is_open()) return nullptr;

    std::vector<ListNode*> nodes;
    std::vector<int> rIdxs;
    std::string str;

    while(std::getline(file, str))
    {
        size_t idxSeparator = str.find_last_of(';');
        if (idxSeparator == std::string::npos) continue;

        std::string dataStr = str.substr(0, idxSeparator);
        std::string rIdxStr = str.substr(idxSeparator + 1);
        int rIdx = std::stoi(rIdxStr);

        ListNode* node = new ListNode();
        node->data = dataStr;

        nodes.push_back(node);
        rIdxs.push_back(rIdx);
    }

    if (nodes.empty()) return nullptr;

    for(size_t i = 0; i < nodes.size(); ++i)
    {
        auto& node = nodes[i];
        node->prev = (i) ? nodes[i - 1] : nullptr;
        node->next = (i + 1 < nodes.size()) ? nodes[i + 1] : nullptr;

        int idx = rIdxs[i];
        node->rand = (idx >= 0 && static_cast<size_t>(idx) < nodes.size()) ? nodes[idx] : nullptr;
    }
    return nodes[0];
}

void serializeList(ListNode* head, const std::string& fName) 
{
    std::ofstream file(fName, std::ios::binary);
    if (!file.is_open()) return;

    std::vector<ListNode*> nodes;
    ListNode* cur = head;
    while (cur != nullptr) 
    {
        nodes.push_back(cur);
        cur = cur->next;
    }

    std::unordered_map<ListNode*, int> nodeToIdx;
    for (size_t i = 0; i < nodes.size(); ++i) nodeToIdx[nodes[i]] = static_cast<int>(i);

    uint64_t nodeCnt = nodes.size();
    file.write(reinterpret_cast<char*>(&nodeCnt), sizeof(nodeCnt));

    for (size_t i = 0; i < nodes.size(); ++i) 
    {
        ListNode* node = nodes[i];

        uint64_t dataLen = node->data.size();
        file.write(reinterpret_cast<char*>(&dataLen), sizeof(dataLen));
        file.write(node->data.c_str(), dataLen);

        int64_t randIndex = -1;
        if (node->rand != nullptr) 
        {
            auto it = nodeToIdx.find(node->rand);
            if (it != nodeToIdx.end()) randIndex = it->second;
        }
        file.write(reinterpret_cast<char*>(&randIndex), sizeof(randIndex));
    }

    file.close();
}

ListNode* deserializeList(const std::string& fName) {
    std::ifstream file(fName, std::ios::binary);
    if (!file.is_open()) return nullptr;

    uint64_t nodeCnt = 0;
    file.read(reinterpret_cast<char*>(&nodeCnt), sizeof(nodeCnt));

    if (nodeCnt == 0) { file.close(); return nullptr; }

    std::vector<ListNode*> nodes(nodeCnt);
    std::vector<int64_t> rIdxs(nodeCnt);

    for (uint64_t i = 0; i < nodeCnt; ++i) {
        uint64_t dataLen = 0;
        file.read(reinterpret_cast<char*>(&dataLen), sizeof(dataLen));

        std::string data;
        data.resize(dataLen);
        file.read(&data[0], dataLen);

        int64_t rIdx = 0;
        file.read(reinterpret_cast<char*>(&rIdx), sizeof(rIdx));

        ListNode* node = new ListNode();
        node->data = data;

        nodes[i] = node;
        rIdxs[i] = rIdx;
    }

    file.close();

    for (uint64_t i = 0; i < nodeCnt; ++i) {
        auto& node = nodes[i];
        node->prev = (i) ? nodes[i - 1] : nullptr;
        node->next = (i + 1 < nodes.size()) ? nodes[i + 1] : nullptr;

        uint64_t idx = rIdxs[i];
        node->rand = (idx >= 0 && idx < nodeCnt) ? nodes[idx] : nullptr;
    }

    return nodes.empty() ? nullptr : nodes[0];
}

void freeList(ListNode* head) {
    while (head != nullptr) {
        ListNode* next = head->next;
        delete head;
        head = next;
    }
}

int main() {
    ListNode* head = readListFromFile("inlet.in");

    /*
    * Вывод входного файла
    ListNode* fCur = head;
    while (fCur != nullptr) { std::cout << fCur->data << std::endl; fCur = fCur->next; }
    */

    serializeList(head, "outlet.out");

    ListNode* deserializedHead = deserializeList("outlet.out");

    /*
    * Вывод выходного файла
    ListNode* sCur = deserializedHead;
    while (sCur != nullptr) { std::cout << sCur->data << std::endl; sCur = sCur->next; }
    */

    freeList(head);
    freeList(deserializedHead);

    return 0;
}