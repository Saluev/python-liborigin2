#include "OriginObj.h"

const double *Origin::getDoubleFromVariant(const variant *v) {
    return boost::get<double, double, string>(v);
}

const string *Origin::getStringFromVariant(const variant *v) {
    return boost::get<string, double, string>(v);
}
