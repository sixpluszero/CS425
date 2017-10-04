#include <string>
using namespace std;
class VMNode{
public:
    string ip; //IP Address
    long long join_timestamp; // Timestamp when joining.
    int id; // Position on virtual ring;
    string toString();
    VMNode(string _ip, long long _join_ts, int _id);
    VMNode(string _ip, long long _join_ts);
    VMNode(const VMNode &tmp);
    VMNode();
};
