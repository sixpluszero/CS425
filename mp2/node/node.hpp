#include <string>

class Node{
public:
	string ip; //IP Address
	long long join_timestamp; // Timestamp when joining.
    long long timestamp; // Last HB TimeStamp;
		int id; // Position on virtual ring;
    string toString();
}
