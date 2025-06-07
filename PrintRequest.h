#ifndef PRINTREQUEST_H
#define PRINTREQUEST_H

#include <string>
#include <vector>

using namespace std;

// ساختاری برای نگهداری اطلاعات یک متغیر که باید چاپ شود
struct PrintVariable {
    char type;    // 'V' for Voltage, 'I' for Current
    string id;    // شماره گره (به صورت رشته) یا نام المان
};

#endif //PRINTREQUEST_H
