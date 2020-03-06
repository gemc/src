#ifndef  GEMC_CONVENTIONS_H
#define  GEMC_CONVENTIONS_H  1

// JSON TAGS
#define JSONTAGNAME "jsonTagName"
#define JSONTAGDESC "jsonTagDescription"
#define JSONTAGDFLT "jsonTagDefault"


#define IMPORTSTRING "importJcards"

#define FILENOTFOUND "fileNotFound"
#define NODEFAULT    "noDefault"

#define SILENTVERBOSITY  0
#define SUMMARYVERBOSITY 1
#define DETAILSERBOSITY  2
#define SAYALLVERBOSITY  3

// to be filled by libraries utilizing goptions
namespace goptions {

}

// gutilities namespace:
#include <vector>
#include <map>
using namespace std;

namespace gutilities {
	template<class KEY, class VALUE>
	vector<KEY> getKeys(const map<KEY, VALUE>& map)
	{
		vector<KEY> keys(map.size());
		for (const auto& it : map)
			keys.push_back(it.first);
		return keys;
	}
}


#endif

