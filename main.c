#include "d:/Desktop/all.h"//万能头文件
#define M 3
using namespace std;
class STU {
public:
	int stuId;    // 学生id
	string stuName;  // 学生姓名
	string stuDept;  // 学生院系
	STU(int id, string name, string dept)
		: stuId(id), stuName(name), stuDept(dept)  
	{
	}
    STU() {};
   friend ostream& operator<<(ostream& os, const STU& stu);
    

};

ostream& operator<<(ostream& os, const STU& stu) {
    os << "Student ID: " << stu.stuId << ", Name: " << stu.stuName << ", Department: " << stu.stuDept;
    return os;
}
class BPlusNode {//b+树节点
public:
	bool isleaf;//是否是叶节点
	BPlusNode* pre;//前,只对叶节点生效
	BPlusNode* next;//后，只对叶节点生效
	BPlusNode* parent;//父亲节点
	int num;//key数量
	vector<int>keys;//存储键值
	vector<BPlusNode*>child;//孩子节点指针列表
	vector<STU>data;//存储数据，只有子节点有，与键值按顺序1-1对应
	BPlusNode(BPlusNode* parent) :isleaf(1),pre(nullptr),next(nullptr),num(0){};
	int split();//节点分裂
	int insert(int key, STU data);//插入数据
	int deletes(int key);//删除键值
	STU* search(int key);//查找数据
	int update(int key, STU data);//更新数据
    int merge();//节点合并
    BPlusNode* getLeftBro();//获得其左兄弟
    BPlusNode* getRightBro();//获得其右兄弟
};


class BPlusTree {
public:
    static BPlusNode* root;//B+树根节点
	static int insert(int key,STU data);//在树中插入数据
    static int update(int key, STU data);//更新数据
    static int deletes(int key);//删除数据
    static STU* search(int key);//查找数据
    static BPlusNode* first;//指向B+树的最下层链表头指针
    static void create();//创建B+树
    static void show(BPlusNode*root);
    static void Print();
};
BPlusNode* BPlusNode::getLeftBro() {
    if (!this->parent)return nullptr;
    BPlusNode* parent = this->parent;
    int i = 0;
    int n = parent->child.size();
    for (i;i<n;i++) {
        if (parent->child[i] == this) {
            if (i > 0) {
                return parent->child[i - 1];
            }
            else {
            
                return nullptr;
            }
        }
    }
}

BPlusNode* BPlusNode::getRightBro() {
    if (!this->parent)return nullptr;
    BPlusNode* parent = this->parent;
    int i = 0;
    int n = parent->child.size();
    for (i; i < n-1; i++) {
        if (parent->child[i] == this) {
                return parent->child[i+1];
        }
    }
    return nullptr;
}


int BPlusNode::insert(int key,STU data) {
	auto it = lower_bound(keys.begin(), keys.end(), key);
	int index = it - keys.begin();//二分查找找到正确的位置
    if (index<keys.size()&&keys[index] == key)return 0;//如果键值已经存在，插入失败返回0
	if (isleaf) {//如果是叶节点插入键值和数据 
		keys.insert(keys.begin() + index, key);//在该位置插入对应键值
		this->data.insert(this->data.begin() + index, data);//对应位置插入数据
		num++;
        if (index == keys.size() - 1) {//如果插入的是最后一个位置，意味着最大值发生变化，迭代向上更新父节点
            BPlusNode* currentNode = this;
            while (currentNode->parent != nullptr) {
                // 找到当前节点在父节点child列表中的索引
                int childIndex = std::find(currentNode->parent->child.begin(), currentNode->parent->child.end(), currentNode) - currentNode->parent->child.begin();
                // 检查当前节点是否是其父节点的最后一个子节点
                currentNode->parent->keys[childIndex] = key;
                if (childIndex!=currentNode->parent->keys.size()-1) {
                    // 不是最后一个子节点，不需要再向上迭代，则退出
                    break; 
                }
                else {
                    // 是最后一个子节点，需要向上更新。
                    currentNode = currentNode->parent;
                }
            }
        }
		if (num > M) { //当前节点数大于M需要递归向上分裂
            BPlusNode* node = this;
            while (node != nullptr && node->split() == 1) {
                node = node->parent;
            }
		}
		return 1;//插入成功 
	}
	else {//非叶结点不会直接插入，而是向子节点递归
        if (index == keys.size())index--;
		BPlusNode* childNode = child[index];
		return childNode->insert(key, data);
	}
}


int BPlusNode::split() {
    if (this->num <= M)return 0;//如果数量没有违反m阶b+树直接返回
    BPlusNode* newNode = new BPlusNode(nullptr);
    int midIndex = ceil(num / 2.0);
    newNode->isleaf = this->isleaf;
    /*处理本层*/
    if (isleaf) {//如果是叶节点不需要处理child,需要处理key和data
        // 如果是叶节点
        newNode->keys.assign(keys.begin() + midIndex, keys.end());//分裂出的新节点在原节点的右侧
        newNode->data.assign(data.begin() + midIndex, data.end());
        keys.resize(midIndex);
        data.resize(midIndex);
        // 更新链表指针

        BPlusNode* tmp = this->next;//存储原本节点的下一个节点
        newNode->pre = this;
        this->next = newNode;
        newNode->next = tmp;
        if (tmp) {
            tmp->pre = newNode;
        }
        newNode->num = newNode->keys.size();//更新num值
        this->num = keys.size();
        newNode->parent = this->parent;
    }
    else {
        // 对于内部节点,没有next和pre指针
        newNode->keys.assign(keys.begin() + midIndex , keys.end()); 
        newNode->child.assign(child.begin() + midIndex , child.end());
        keys.resize(midIndex); // 包括中间键在内节点中
        child.resize(midIndex);
        for (auto& childNode : newNode->child) {
            childNode->parent = newNode;
        }

        newNode->parent = this->parent;
        newNode->num = newNode->keys.size();
        this->num = keys.size();
    }
//向上更新父节点key值
    /*处理上一层，即父节点那一层*/
    if (this->parent == nullptr) {
        // 如果当前节点是根节点，则需要创建一个新的根节点
        BPlusNode* newRoot = new BPlusNode(nullptr);
        BPlusTree::root = newRoot;//更改根节点
        newRoot->isleaf = false;
        newRoot->keys.push_back(this->keys[keys.size() - 1]);
        newRoot->keys.push_back(newNode->keys[newNode->keys.size()-1]);
        newRoot->child.push_back(this);
        newRoot->child.push_back(newNode);
        newRoot->num = 2;
        this->parent = newRoot;
        newNode->parent = newRoot;
       
    }
    else {//当前节点不是根

        BPlusNode* parent = this->parent;
        int newKey = this->keys.back(); // 新节点中的最大键
        auto it = std::lower_bound(parent->keys.begin(), parent->keys.end(), newKey);
        int index = it - parent->keys.begin();
        // 在父节点中插入新键
        parent->keys.insert(parent->keys.begin() + index, newKey);
        // 确保子节点列表也被更新以包含新节点
        parent->child.insert(parent->child.begin() + index+1, newNode);
        parent->num++;
        // 更新新节点的父指针
        newNode->parent = parent;
    }

    return 1; //返回1，表示当前节点完成分裂
}

int BPlusNode::deletes(int key) {//删除当前节点对应键值
    if (isleaf) {
        auto it = lower_bound(keys.begin(), keys.end(), key);
        if (it != keys.end() && *it == key) { // 找到了键值
            bool flag = 0;
            int index = it - keys.begin();
            if (index == keys.size() - 1) {
                flag = 1;//删除的是最后一个
            }
            keys.erase(it); // 删除键值
            data.erase(data.begin() + index); // 删除对应的数据
            num--; // 更新键值数量
            if (flag) {//自下向上更新
                BPlusNode* currentNode = this;
                int tmpkey;
                if (!keys.empty()) {
                  tmpkey = keys.back();
                }
                while (currentNode->parent != nullptr) {//当前节点在父节点中的索引
                   int childIndex= std::find(currentNode->parent->child.begin(), currentNode->parent->child.end(), currentNode) - currentNode->parent->child.begin();
                   currentNode->parent->keys[childIndex] = tmpkey;
                   if (childIndex == currentNode->parent->child.size()-1) {
                       currentNode = currentNode->parent;
                   }
                   else {
                       break;
                   }
                }
            }
            //如果当前的key数量小于m/2需要向兄弟节点借，尽量先从右兄弟借，两个兄弟都不够借则需要合并
            if (this->keys.size()<ceil(M/2.0)&&parent) {//右兄弟
                BPlusNode* t = this;
                while (t!=nullptr && t->merge()) {
                    t = t->parent;
                }
            }
            return 1; // 成功删除
        }
        return 0; // 没有找到键值
    }
    else {
        auto it = lower_bound(keys.begin(), keys.end(), key);
        int index = it - keys.begin();
        if (it == keys.end())return 0;
        return child[index]->deletes(key); // 递归删除
       
    }

}

int BPlusNode::merge() {//处理节点的借用或者合并
    if (keys.size() >= ceil(M / 2.0)) {//不需要合并
        return 0;
    }
    if (this == BPlusTree::root) {
        if (this->child.size() >= 2) {//根节点的孩子大于等于2即可
            return 0;
        }
        else {//否则更换根节点
            BPlusTree::root = this->child[0];//当前节点的孩子成为根节点
            this->child[0]->parent = nullptr;
            delete this;
            return 0;
        }
    }
    BPlusNode* right = getRightBro();//右兄弟
    BPlusNode* left = getLeftBro();//左兄弟
    if (isleaf) {//如果当前节点是叶节点
        //从右节点借
        if (right != nullptr && right->keys.size() > ceil(M / 2.0)) {
            this->keys.emplace_back(right->keys[0]);
            this->data.emplace_back(right->data[0]);
            right->keys.erase(right->keys.begin());
            right->data.erase(right->data.begin());
            this->num++;
            right->num--;
            //向上更新父节点
            int key = keys.back();
            BPlusNode* currentNode = this;
            while (currentNode->parent != nullptr) {
                // 找到当前节点在父节点child列表中的索引
                int childIndex = std::find(currentNode->parent->child.begin(), currentNode->parent->child.end(), currentNode) - currentNode->parent->child.begin();
                // 检查当前节点是否是其父节点的最后一个子节点
                currentNode->parent->keys[childIndex] = key;
                if (childIndex == currentNode->parent->keys.size() - 1) {//当前节点是父节点的最后一个孩子，需要迭代向上
                    currentNode = currentNode->parent;
                }
                else {
                    break;
                }
            }
            return 0;
        }//从左兄弟借
        else if (left != nullptr && left->keys.size() > ceil(M / 2.0)) {
            this->num++;
            left->num--;
            this->keys.insert(this->keys.begin(), left->keys.back());
            this->data.insert(this->data.begin(), left->data.back());
            left->keys.pop_back();
            left->data.pop_back();
            int key = left->keys.back();
            BPlusNode* currentNode = left;
            while (currentNode->parent != nullptr) {
                // 找到当前节点在父节点child列表中的索引
                int childIndex = std::find(currentNode->parent->child.begin(), currentNode->parent->child.end(), currentNode) - currentNode->parent->child.begin();
                // 检查当前节点是否是其父节点的最后一个子节点
                currentNode->parent->keys[childIndex] = key;
                if (childIndex == currentNode->parent->keys.size() - 1) {//当前节点是父节点的最后一个孩子，需要迭代向上
                    currentNode = currentNode->parent;
                }
                else {
                    break;
                }
            }
            return 0;
        }
        else {//需要合并
            if (left && right) {//左右节点都可以合并则优先选择填充较少的进行合并以降低插入时拆分的几率。
                if (left->keys.size() < right->keys.size()) {//与左节点合并
                    for (auto it : this->child) {
                        it->parent = left;
                    }
                    left->next = next;
                    if (next)next->pre = left;
                    //当前节点在父节点中的迭代器
                    auto it = std::find(parent->child.begin(), parent->child.end(), this);
                    //int index = find(parent->child.begin(), parent->child.end(), this) - parent->child.begin();
                    auto it2 = find(parent->keys.begin(), parent->keys.end(), this->keys.back());
                    parent->keys.erase(it2);//从父节点中去除键值和孩子
                    parent->child.erase(it);
                    left->keys.insert(left->keys.end(), this->keys.begin(), this->keys.end());
                    left->data.insert(left->data.end(), this->data.begin(), this->data.end());
                    //向上更新键值
                    int key = left->keys.back();//
                    BPlusNode* currentNode = left;
                    while (currentNode->parent != nullptr) {
                        // 找到当前节点在父节点child列表中的索引
                        int childIndex = std::find(currentNode->parent->child.begin(), currentNode->parent->child.end(), currentNode) - currentNode->parent->child.begin();
                        // 检查当前节点是否是其父节点的最后一个子节点
                        currentNode->parent->keys[childIndex] = key;
                        if (childIndex == currentNode->parent->keys.size() - 1) {//当前节点是父节点的最后一个孩子，需要迭代向上
                            currentNode = currentNode->parent;
                        }
                        else {
                            break;
                        }
                    }
                  //  delete this;
                    return 1;
                }
                else {//与右节点合并

                    for (auto it : right->child) {
                        it->parent = this;
                    }
                    this->next = this->next->next;
                    if (this->next&&this->next->next) {
                        this->next->next->pre = this;
                    }
                    auto it = find(parent->child.begin(), parent->child.end(), right);
                    auto it2 = find(parent->keys.begin(), parent->keys.end(), right->keys.back());
                    parent->child.erase(it);
                    parent->keys.erase(it2);
                    keys.insert(keys.end(), right->keys.begin(), right->keys.end());
                    data.insert(data.end(), right->data.begin(), right->data.end());
                    //向上更新键值
                    int key = right->keys.back();
                    BPlusNode* currentNode = this;
                    while (currentNode->parent != nullptr) {
                        // 找到当前节点在父节点child列表中的索引
                        int childIndex = std::find(currentNode->parent->child.begin(), currentNode->parent->child.end(), currentNode) - currentNode->parent->child.begin();
                        // 检查当前节点是否是其父节点的最后一个子节点
                        currentNode->parent->keys[childIndex] = key;
                        if (childIndex == currentNode->parent->keys.size() - 1) {//当前节点是父节点的最后一个孩子，需要迭代向上
                            currentNode = currentNode->parent;
                        }
                        else {
                            break;
                        }
                    }
                   // delete right;
                    return 1;

                }

            }

            else if (right) {//右

                for (auto it : right->child) {
                    it->parent = this;
                }
                this->next = this->next->next;
                if (this->next&&this->next->next) {
                    this->next->next->pre = this;
                }
                auto it = find(parent->child.begin(), parent->child.end(), right);
                auto it2 = find(parent->keys.begin(), parent->keys.end(), right->keys[right->keys.size() - 1]);
                parent->child.erase(it);
                parent->keys.erase(it2);
                keys.insert(keys.end(), right->keys.begin(), right->keys.end());
                data.insert(data.end(), right->data.begin(), right->data.end());
                //向上更新键值
                int key = right->keys[right->keys.size() - 1];
                BPlusNode* currentNode = this;
                while (currentNode->parent != nullptr) {
                    // 找到当前节点在父节点child列表中的索引
                    int childIndex = std::find(currentNode->parent->child.begin(), currentNode->parent->child.end(), currentNode) - currentNode->parent->child.begin();
                    // 检查当前节点是否是其父节点的最后一个子节点
                    currentNode->parent->keys[childIndex] = key;
                    if (childIndex == currentNode->parent->keys.size() - 1) {//当前节点是父节点的最后一个孩子，需要迭代向上
                        currentNode = currentNode->parent;
                    }
                    else {
                        break;
                    }
                }
               // delete right;
                return 1;
            }
            else {//左节点

                for (auto it : this->child) {
                    it->parent = left;
                }
                left->next = next;
                if (next)next->pre = left;
                //当前节点在父节点中的迭代器
                auto it = std::find(parent->child.begin(), parent->child.end(), this);
                //int index = find(parent->child.begin(), parent->child.end(), this) - parent->child.begin();
                auto it2 = find(parent->keys.begin(), parent->keys.end(), this->keys[this->keys.size() - 1]);
                parent->keys.erase(it2);//从父节点中去除键值和孩子
                parent->child.erase(it);
                left->keys.insert(left->keys.end(), this->keys.begin(), this->keys.end());
                left->data.insert(left->data.end(), this->data.begin(), this->data.end());
                //向上更新键值
                int key = left->keys[left->keys.size() - 1];//
                BPlusNode* currentNode = left;
                while (currentNode->parent != nullptr) {
                    // 找到当前节点在父节点child列表中的索引
                    int childIndex = std::find(currentNode->parent->child.begin(), currentNode->parent->child.end(), currentNode) - currentNode->parent->child.begin();
                    // 检查当前节点是否是其父节点的最后一个子节点
                    currentNode->parent->keys[childIndex] = key;
                    if (childIndex == currentNode->keys.size() - 1) {//当前节点是父节点的最后一个孩子，需要迭代向上
                        currentNode = currentNode->parent;
                    }
                    else {
                        break;
                    }
                }
                //delete this;
                return 1;
            }

        }

    }
    else {//非叶节点，向右节点借
        if (right != nullptr && right->keys.size() > ceil(M / 2)) {

            right->child[0]->parent = this;
            this->keys.emplace_back(right->keys[0]);
            this->child.emplace_back(right->child[0]);
            right->keys.erase(right->keys.begin());
            right->child.erase(right->child.begin());
            this->num++;
            right->num--;
            //向上更新父节点
            int key = keys[keys.size() - 1];
            BPlusNode* currentNode = this;
            while (currentNode->parent != nullptr) {
                // 找到当前节点在父节点child列表中的索引
                int childIndex = std::find(currentNode->parent->child.begin(), currentNode->parent->child.end(), currentNode) - currentNode->parent->child.begin();
                // 检查当前节点是否是其父节点的最后一个子节点
                currentNode->parent->keys[childIndex] = key;
                if (childIndex == currentNode->parent->keys.size() - 1) {//当前节点是父节点的最后一个孩子，需要迭代向上
                    currentNode = currentNode->parent;
                }
                else {
                    break;
                }
            }
            return 0;

        }
        else if (left != nullptr && left->keys.size() > ceil(M / 2)) {
            left->child.back()->parent = this;
            this->child.insert(this->child.begin(), left->child.back());
            this->keys.insert(this->keys.begin(), left->keys.back());
            left->child.pop_back();
            left->keys.pop_back();
            left->num--;
            this->num++;
            int key = left->keys.back();


            BPlusNode* currentNode = left;
            while (currentNode->parent != nullptr) {
                // 找到当前节点在父节点child列表中的索引
                int childIndex = std::find(currentNode->parent->child.begin(), currentNode->parent->child.end(), currentNode) - currentNode->parent->child.begin();
                // 检查当前节点是否是其父节点的最后一个子节点
                currentNode->parent->keys[childIndex] = key;
                if (childIndex == currentNode->parent->keys.size() - 1) {//当前节点是父节点的最后一个孩子，需要迭代向上
                    currentNode = currentNode->parent;
                }
                else {
                    break;
                }
            }
            return 0;

        }
        else {//需要合并
            //与左节点合并
            if (left) {

                    for (auto it : this->child) {
                    it->parent = left;
                }

                    left->next = next;
                    if (next)next->pre = left;

                //当前节点在父节点中的迭代器
                auto it = std::find(parent->child.begin(), parent->child.end(), this);
                //int index = find(parent->child.begin(), parent->child.end(), this) - parent->child.begin();
                auto it2 = find(parent->keys.begin(), parent->keys.end(), this->keys[this->keys.size() - 1]);
                parent->keys.erase(it2);//从父节点中去除键值和孩子
                parent->child.erase(it);
                left->keys.insert(left->keys.end(), this->keys.begin(), this->keys.end());
                left->child.insert(left->child.end(), this->child.begin(), this->child.end());
                //向上更新键值
                int key = left->keys[left->keys.size() - 1];//
                BPlusNode* currentNode = left;
                while (currentNode->parent != nullptr) {
                    // 找到当前节点在父节点child列表中的索引
                    int childIndex = std::find(currentNode->parent->child.begin(), currentNode->parent->child.end(), currentNode) - currentNode->parent->child.begin();
                    // 检查当前节点是否是其父节点的最后一个子节点
                    currentNode->parent->keys[childIndex] = key;
                    if (childIndex == currentNode->keys.size() - 1) {//当前节点是父节点的最后一个孩子，需要迭代向上
                        currentNode = currentNode->parent;
                    }
                    else {
                        break;
                    }
                }
                //delete this;
                return 1;
            }
            else {//右节点合并


                for (auto it : right->child) {
                    it->parent = this;
                }

                this->next = this->next->next;
                if (this->next&&this->next->next) {
                    this->next->next->pre = this;
                }
                auto it = find(parent->child.begin(), parent->child.end(), right);
                auto it2 = find(parent->keys.begin(), parent->keys.end(), right->keys[right->keys.size() - 1]);
                parent->child.erase(it);
                parent->keys.erase(it2);
                keys.insert(keys.end(), right->keys.begin(), right->keys.end());
                child.insert(child.end(), right->child.begin(), right->child.end());
                //向上更新键值
                int key = right->keys[right->keys.size() - 1];
                BPlusNode* currentNode = this;
                while (currentNode->parent != nullptr) {
                    // 找到当前节点在父节点child列表中的索引
                    int childIndex = std::find(currentNode->parent->child.begin(), currentNode->parent->child.end(), currentNode) - currentNode->parent->child.begin();
                    // 检查当前节点是否是其父节点的最后一个子节点
                    currentNode->parent->keys[childIndex] = key;
                    if (childIndex == currentNode->parent->keys.size() - 1) {//当前节点是父节点的最后一个孩子，需要迭代向上
                        currentNode = currentNode->parent;
                    }
                    else {
                        break;
                    }
                }

                //delete right;
                return 1;

            }

        }
       
    }
    return 1;
}


STU* BPlusNode::search(int key) {//查找
    // 首先检查当前节点是否为叶节点
    if (isleaf) {
        for (size_t i = 0; i < keys.size(); ++i) {
            if (keys[i] == key) {
                // 找到了键，返回对应的数据
               
                return new STU(data[i].stuId,data[i].stuName,data[i].stuDept);
            }
        }
        return nullptr;//未找到返回空
    }
    else {
        // 如果当前节点是内部节点，需要决定遍历哪个子节点
        for (size_t i = 0; i < keys.size(); ++i) {
            if (key <=keys[i]) {
                return child[i]->search(key);
            }
        }
        // 如果所有的键都小于key，那么遍历最右侧的子节点
        return nullptr;
    }
}

int BPlusNode::update(int key,STU stu) {//更新
    if (isleaf) {
        for (size_t i = 0; i < keys.size(); ++i) {
            if (keys[i] == key) {
                // 找到了键，对其更新
                data[i] = stu;
                return 1;
                
            }
        }
        return 0;//未更新成功则返回0
    }
    else {
        
        // 如果当前节点是内部节点，需要决定遍历哪个子节点
        for (size_t i = 0; i < keys.size(); ++i) {
            if (key <= keys[i]) {
                return child[i]->update(key,stu);
            }
        }
        return 0;
    }
}
void BPlusTree::create() {//创建一颗B+树
    BPlusTree::root = new BPlusNode(nullptr);
    BPlusTree::first = BPlusTree::root;//初始化first
    root->isleaf = 1;
    root->num = 0;
}

int BPlusTree::insert(int key,STU stu) {//插入数据
    return root->insert(key,stu);
}

STU* BPlusTree::search(int key) {//查找数据

    return root->search(key);
}

int BPlusTree::update(int key, STU stu) {//更新数据

    return root->update(key, stu);
}

int BPlusTree::deletes(int key) {//删除数据
    return root->deletes(key);

}

void BPlusTree::Print() {
    BPlusNode* t = BPlusTree::first;
    while (t) {
        cout << "[";
        int n = t->data.size();
        for (int i = 0; i < n; i++) {
            cout << t->data[i];
            if (i != n - 1)cout << ",";
        }
        cout << "]" << endl;
        t = t->next;
    }
}

BPlusNode* BPlusTree::root = nullptr;
BPlusNode* BPlusTree::first = nullptr;
/*
void BPlusTree::show(BPlusNode* root) {
    if (root == nullptr) return; // 空树直接返回

    std::queue<BPlusNode*> queue; // 用于层次遍历的队列
    std::queue<int> levels; // 用于存储每个节点的层级信息
    queue.push(root);
    levels.push(0); // 根节点层级为0
    queue.push(nullptr); // 加入一个nullptr作为第一层结束的标志
    levels.push(0); // 对应于nullptr的层级（不会被使用）

    int currentLevel = -1; // 当前正在打印的层级

    while (!queue.empty()) {
        BPlusNode* currentNode = queue.front();
        queue.pop();
        int nodeLevel = levels.front();
        levels.pop();

        if (currentNode == nullptr) {
            // 到达一层的末尾
            std::cout << "\n"; // 换行以区分不同的层
            if (!queue.empty()) {
                queue.push(nullptr); // 如果队列不为空，则添加新的层结束标志
                levels.push(nodeLevel + 1); // 下一个nullptr对应的层级
            }
            continue; // 跳过下面的处理，直接进入下一轮循环
        }

        if (nodeLevel != currentLevel) {
            currentLevel = nodeLevel;
            std::cout << "Level " << currentLevel << ": "<<endl;
        }

        std::cout << "["; // 节点开始标志
        // 检查是否为叶节点
        if (currentNode->isleaf) {
            // 叶节点：打印键值和数据
            for (int i = 0; i < currentNode->keys.size(); ++i) {
                std::cout << "(" << currentNode->keys[i] << ", " << currentNode->data[i] << ")";
                if (i < currentNode->keys.size() - 1) std::cout << ", "; // 在同一节点的不同键值对之间添加分隔符
            }
        }
        else {
            // 非叶节点：仅打印键值
            for (int i = 0; i < currentNode->keys.size(); ++i) {
                std::cout << currentNode->keys[i];
                if (i < currentNode->keys.size() - 1) std::cout << ", "; // 在同一节点的不同键之间添加分隔符
            }
            // 将子节点加入队列
            for (BPlusNode* child : currentNode->child) {
                if (child != nullptr) {
                    queue.push(child);
                    levels.push(nodeLevel + 1); // 子节点层级是当前节点层级加1
                }
            }
        }
        std::cout << "] "<<endl; // 节点结束标志
    }
}
*/
void BPlusTree::show(BPlusNode* root) {
    if (root == nullptr) return;

    std::queue<BPlusNode*> queue;
    std::queue<int> levelQueue;
    queue.push(root);
    levelQueue.push(0); // 根节点在第0层

    int currentLevel = 0;
    std::cout << "Level " << currentLevel << ":\n";

    while (!queue.empty()) {
        BPlusNode* node = queue.front();
        int nodeLevel = levelQueue.front();
        queue.pop();
        levelQueue.pop();

        if (nodeLevel > currentLevel) {
            currentLevel = nodeLevel;
            std::cout << "\nLevel " << currentLevel << ":\n";
        }

        std::cout << "[";
        for (size_t i = 0; i < node->keys.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << node->keys[i];
        }
        std::cout << "] ";

        if (!node->isleaf) {
            for (auto child : node->child) {
                if (child != nullptr) {
                    queue.push(child);
                    levelQueue.push(nodeLevel + 1); // 子节点层级是当前节点层级加1
                }
            }
        }
    }
    std::cout << std::endl; // 在遍历结束后换行
}

int main() {
    BPlusTree::create();
   
    while (1) { 
        system("CLS");
        cout << "请输入你要进行的操作:" << endl;
        cout << "-------------------------------------------------------" << endl;
        cout << "1.插入数据" << endl;
        cout << "2.删除数据" << endl;
        cout << "3.更新数据" << endl;
        cout << "4.查询数据" << endl;
        cout << "5.打印所有数据" << endl;
        cout << "0.退出程序" << endl;
        int in;
        cin >> in;
        int key;
        int stuId;
        string  stuName, stuDept;
        STU* stu = nullptr;
        switch (in) {
        case 0:exit(0);
            break;
        case 1:
            cout << "请依次输入key、stuId、stuName、stuDept：" << endl;
            
            cin >> key >> stuId >>stuName >> stuDept;
            if (BPlusTree::insert(key, *(new STU(stuId, stuName, stuDept)))) {
                cout << "插入成功" << endl;
              
            }
            else {
                cout << "插入失败" << endl; 
            };
            BPlusTree::show(BPlusTree::root);
            break;
        case 2:
            cout << "请输入key值" << endl;
            cin >> key;
            if (BPlusTree::deletes(key)) {
                cout << "删除成功" << endl;
            }
            else {
                cout << "删除失败" << endl;
            };
            BPlusTree::show(BPlusTree::root);
            break;
        case 3:
            cout << "请依次输入key、stuId、stuName、stuDept：" << endl;
            cin >> key >> stuId >> stuName >> stuDept;
            if (BPlusTree::update(key, *(new STU(stuId, stuName, stuDept)))) {
                cout << "更新成功" << endl;
            }
            else {
                cout << "更新失败" << endl;
            };
            BPlusTree::show(BPlusTree::root);
            break;
        case 4:
            cout << "请输入key值" << endl;
            cin >> key;
            stu = (BPlusTree::search(key));
            if (stu)cout << *stu << endl;
            else cout << "未查询到" << endl;
            //BPlusTree::show(BPlusTree::root);
            break;
        case 5:
            BPlusTree::Print();
            break;
        default:
            cout << "输入不合法" << endl;
        }
        system("pause");
    }
    return 0;
}
