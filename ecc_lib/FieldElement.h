//
// Created by preston on 10/1/2023.
//

#ifndef ECC_FIELDELEMENT_H
#define ECC_FIELDELEMENT_H

#include "integer.h"

using namespace std;

class FieldElement {
public:
    FieldElement(const integer& num, const integer& prime);

    FieldElement operator+(FieldElement &other);
    FieldElement operator-(FieldElement &other);
    FieldElement operator*(FieldElement &other);
    FieldElement operator*(const integer& other);
    FieldElement operator/(FieldElement &other);
    FieldElement power(const integer& power);

    friend bool operator==(const FieldElement& lhs, const FieldElement& rhs);
    friend bool operator!=(const FieldElement& lhs, const FieldElement& rhs);
    friend ostream& operator<<( ostream& os, const FieldElement& a );
private:
    integer num;
    integer prime;
};

#endif //ECC_FIELDELEMENT_H
