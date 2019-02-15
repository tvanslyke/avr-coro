#include <typeinfo>



void t() {
	const std::type_info& ti = typeid(int);
}
