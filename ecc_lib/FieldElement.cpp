//
// Created by preston on 10/1/2023.
//
#include <stdexcept>
#include "FieldElement.h"

FieldElement::FieldElement(const integer& num, const integer& prime) {
    if (num >= prime || num < 0) {
        throw std::invalid_argument("Num is out of range");
    }
    this->num = num;
    this->prime = prime;
}

FieldElement FieldElement::operator+(FieldElement &other) {
    if (this->prime != other.prime) {
        throw std::runtime_error("Cannot add two numbers in different fields");
    }

    integer calculatedAdd = (this->num + other.num) % this->prime;

    return {integer(calculatedAdd), this->prime};
}

FieldElement FieldElement::operator-(FieldElement &other) {
    if (this->prime != other.prime) {
        throw std::runtime_error("Cannot subtract two numbers in different fields");
    }

    integer calculatedAdd = (this->num - other.num) % this->prime;

    return {integer(calculatedAdd), this->prime};
}

FieldElement FieldElement::operator*(FieldElement &other) {
    if (this->prime != other.prime) {
        throw std::runtime_error("Cannot multiply two numbers in different fields");
    }

    integer calculatedAdd = (this->num * other.num) % this->prime;

    return {integer(calculatedAdd), this->prime};
}

FieldElement FieldElement::operator*(const integer& other) {
    integer calculatedAdd = (this->num * other) % this->prime;

    return {integer(calculatedAdd), this->prime};
}

FieldElement FieldElement::operator/(FieldElement &other) {
    if (this->prime != other.prime) {
        throw std::runtime_error("Cannot multiply two numbers in different fields");
    }

    integer calculatedDiv = (this->num * (pow(other.num, this->prime - 2) % this->prime)) % this->prime;

    return {integer(calculatedDiv), this->prime};
}

FieldElement FieldElement::power(const integer &power) {
    integer n = power % (this->prime - 1);
    integer calculatedPow = pow(this->num, n) % this->prime;
    return {calculatedPow, this->prime};
}

bool operator==(const FieldElement &lhs, const FieldElement &rhs) {
    return lhs.num == rhs.num && lhs.num == rhs.num;
}

bool operator!=(const FieldElement &lhs, const FieldElement &rhs) {
    return !(lhs == rhs);
}

ostream &operator<<(ostream &os, const FieldElement &a) {
    os << "FieldElement_" << a.prime << "(" << a.num << ")";
    return os;
}

