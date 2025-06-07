#ifndef PRINTREQUEST_H
#define PRINTREQUEST_H

#include <string>
#include <vector>

using namespace std;

// ساختاری برای نگهداری اطلاعات یک متغیر که باید چاپ شود
// مثلا: type='V', id="1"  یا  type='I', id="R1"
struct PrintVariable {
    char type;    // 'V' for Voltage, 'I' for Current
    string id;    // Node number (as string) or Component name
};

#endif //PRINTREQUEST_H
