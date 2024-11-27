#include <iostream>
#include <cstdint>
#include <string>
#include <stdio.h> // for printf
#include <algorithm> // for std::min
#include <cmath> // for std::abs

enum class RoundingTypes {
    TowardZero,
    TowardInfinity,
    TowardNegInfinity,
    ToNearestEven,
};

struct FirstFourDigits {
    FirstFourDigits(uint16_t number) {
        first = number / 1000;
        second = (number % 1000) / 100;
        third = (number % 100) / 10;
        fourth = number % 10;
    }

    uint8_t first;
    uint8_t second;
    uint8_t third;
    uint8_t fourth;
    bool overflow = false;

    uint16_t operator[](uint8_t index) const {
        switch (index) {
            case 1:
                return first;
            case 2:
                return second;
            case 3:
                return third;
            case 4:
                return fourth;
        }
        throw std::out_of_range("Only indexes 1, 2, 3, 4 are possible");
    }

    void Increment() {
        ++third;
        second += third / 10;
        third %= 10;
        first += second / 10;
        second %= 10;

        if (first == 10) {
            overflow = true;
        }

        first %= 10;
    }

    void ToNearestEven() {
        if (third % 2 != 0) Increment();
    }
};

class PrintingNumber {
public:
    PrintingNumber(uint8_t A, uint8_t B, uint32_t number):
                    A(A), B(B), number(number) {}

    void Print(RoundingTypes round_type) {
        int32_t full_part = static_cast<int64_t>(number >> B) - ((number & 1 << B + A - 1) >> B << 1);
        uint32_t fractional_part = number & (1 << B) - 1;

        bool is_negative = false;
        if (full_part < 0) {
            is_negative = true;
            if (fractional_part) {
                ++full_part;
                fractional_part = (1 << B) - fractional_part;
            }
        }

        uint32_t first_four_digits = static_cast<uint64_t>(fractional_part) * 10000 >> B;
        FirstFourDigits digits(first_four_digits);

        switch (round_type) {
            case RoundingTypes::TowardZero:
                RoundTowardZero(full_part, fractional_part, digits, is_negative);
                break;
            case RoundingTypes::TowardInfinity:
                RoundTowardInfinity(full_part, fractional_part, digits, is_negative);
                break;
            case RoundingTypes::TowardNegInfinity:
                RoundTowardNegInfinity(full_part, fractional_part, digits, is_negative);
                break;
            case RoundingTypes::ToNearestEven:
                RoundToNearestEven(full_part, fractional_part, digits, is_negative);
                break;
        }

        if (full_part == 0 && digits[1] == 0 && digits[2] == 0 && digits[3] == 0) {
            is_negative = false;
        }

        if (is_negative) std::cout << '-';
        std::cout << abs(full_part) << '.' << digits[1] << digits[2] << digits[3];
    }

private:
    const uint8_t A;
    const uint8_t B;
    const uint32_t number;

    void RoundTowardZero(int32_t& full_part, uint32_t fractional_part, FirstFourDigits& first_four_digits, bool is_negative) {

    }

    void RoundTowardInfinity(int32_t& full_part, uint32_t fractional_part, FirstFourDigits& first_four_digits, bool is_negative) {
        if (is_negative || B <= 3) return;
        if ((fractional_part & ((1 << B - 3)) - 1) == 0) return;
        first_four_digits.Increment();
        if (first_four_digits.overflow) {
            ++full_part;
            first_four_digits.overflow = false;
        }
    }

    void RoundTowardNegInfinity(int32_t& full_part, uint32_t fractional_part, FirstFourDigits& first_four_digits, bool is_negative) {
        if (!is_negative || B <= 3) return;
        if ((fractional_part & ((1 << B - 3)) - 1) == 0) return;
        first_four_digits.Increment();
        if (first_four_digits.overflow) {
            --full_part;
            first_four_digits.overflow = false;
        }
    }

    void RoundToNearestEven(int32_t& full_part, uint32_t fractional_part, FirstFourDigits& first_four_digits, bool is_negative) {
        if (first_four_digits[4] > 5) {
            first_four_digits.Increment();
        } else if (first_four_digits[4] < 5) {
            return;
        } else {
            if (B <= 4) {
                first_four_digits.ToNearestEven();
            } else if ((fractional_part & ((1 << B - 4)) - 1) == 0) {
                first_four_digits.ToNearestEven();
            } else {
                first_four_digits.Increment();
            }
        }

        if (first_four_digits.overflow) {
            is_negative ? --full_part: ++full_part;
            first_four_digits.overflow = false;
        }
    }
};


struct BinaryRoundings {
    BinaryRoundings(uint32_t fractional_part, bool is_negative, int B, bool is_further_bits, uint64_t full_part):
                    fractional_part(fractional_part), is_negative(is_negative), B(B), is_further_bits(is_further_bits),
                    full_part(full_part) {
        is_last_bit = fractional_part & 1;
        overflow = false;
    }

    uint32_t fractional_part;
    uint64_t full_part;
    bool is_negative;
    int B;
    bool is_further_bits;

    bool is_last_bit;
    bool overflow;

    uint32_t Increment() {
        fractional_part >>= 1;
        ++fractional_part;
        fractional_part &= (1 << B) - 1; // отбрасываем старший бит
        if (fractional_part == 0) {
            overflow = true;
        }
        return fractional_part;
    }

    uint32_t RoundTowardZero() {
        return fractional_part >> 1;
    }

    uint32_t RoundTowardInfinity() {
        if (is_negative) return RoundTowardZero();

        if (is_further_bits || is_last_bit) {
            return Increment();
        }
        return RoundTowardZero();
    }

    uint32_t RoundTowardNegInfinity() {
        if (!is_negative) return RoundTowardZero();

        if (is_further_bits || is_last_bit) {
            return Increment();
        }
        return RoundTowardZero();
    }

    uint32_t RoundToNearestEven() {
        if (!is_last_bit) return RoundTowardZero();
        if (is_further_bits) {
            return Increment();
        }

        if (B != 0) {
            if (fractional_part & 0b10) {
                return Increment();
            }
            return RoundTowardZero();
        }

        if (fractional_part & 1) {
            if (full_part % 2) {
                overflow = true;
            }
        }

        return RoundTowardZero();
    }
};

class FixedPointNumber {
public:
    FixedPointNumber(uint8_t A, uint8_t B, uint32_t number, RoundingTypes rounding_type): A(A), B(B), number(number), rounding_type(rounding_type) {

    }

    template<typename T>
    T ModularArithmetic(T number, int cutting_bits_number) const {
        return (number << cutting_bits_number >> cutting_bits_number) ;
    }

    PrintingNumber operator+(const FixedPointNumber& rhs) const {
        return PrintingNumber(A, B, ModularArithmetic(number + rhs.number, 32 - A - B));
    }

    PrintingNumber operator-(const FixedPointNumber& rhs) const {
        if (!rhs.number) {
            return PrintingNumber(A, B, number);
        }
        uint32_t negative_rhs_number = (static_cast<uint64_t>(1) << A + B) - rhs.number;


        return PrintingNumber(A, B, ModularArithmetic(number + negative_rhs_number, 32 - A - B));
    }

    PrintingNumber operator*(const FixedPointNumber& rhs) const {
        if (rhs.number * number == 0) {
            return PrintingNumber(A, B, 0);
        }

        int is_lhs_positive = 1;
        int is_rhs_positive = 1;

        int64_t signed_lhs = static_cast<int64_t>(number) - (static_cast<int64_t>(number & 1 << B + A - 1) << 1);
        int64_t signed_rhs = static_cast<int64_t>(rhs.number) - (static_cast<int64_t>(rhs.number & 1 << B + A - 1) << 1);

        if (signed_lhs < 0) is_lhs_positive = -1;
        if (signed_rhs < 0) is_rhs_positive = -1;
        uint64_t lhs64 = std::abs(signed_lhs);
        uint64_t rhs64 = std::abs(signed_rhs);

        bool is_negative = is_lhs_positive * is_rhs_positive == -1;
        uint64_t T = lhs64 * rhs64;

        if (B == 0) {
            if (is_negative) {
                if (A == 32) {
                    T = 0xffffffffffffffff - T + 1;
                } else {
                    T = (static_cast<uint64_t>(1) << A + A) - T;
                }
            }
            return PrintingNumber(A, B, ModularArithmetic(T, 64 - A - B));
        }

        bool is_further_bits = T & (1 << B - 1) - 1;
        uint32_t full_part = T >> B + B;
        uint32_t fractional_part = (T >> B - 1) & (static_cast<uint64_t>(1) << B + 1) - 1;

        BinaryRoundings binary_roundings(fractional_part, is_negative, B, is_further_bits, full_part);
        switch (rounding_type) {
            case RoundingTypes::TowardZero:
                fractional_part = binary_roundings.RoundTowardZero();
                break;
            case RoundingTypes::TowardInfinity:
                fractional_part = binary_roundings.RoundTowardInfinity();
                break;
            case RoundingTypes::TowardNegInfinity:
                fractional_part = binary_roundings.RoundTowardNegInfinity();
                break;
            case RoundingTypes::ToNearestEven:
                fractional_part = binary_roundings.RoundToNearestEven();
                break;
        }


        if (binary_roundings.overflow) {
            ++full_part;
            binary_roundings.overflow = false;
        }

        uint64_t bits = (full_part << B) + fractional_part;

        if (is_negative && bits) {
            bits = (static_cast<uint64_t>(1) << B + A + A) - bits;
        }

        uint32_t result = ModularArithmetic(bits, 64 - A - B);
        return PrintingNumber(A, B, result);
    }

    PrintingNumber operator/(const FixedPointNumber& rhs) const {
        if (!rhs.number) {
            std::cout << "division by zero";
            exit(0);
        }
        int is_lhs_positive = 1;
        int is_rhs_positive = 1;

        int64_t signed_lhs = static_cast<int64_t>(number) - (static_cast<int64_t>(number & 1 << B + A - 1) << 1);
        int64_t signed_rhs = static_cast<int64_t>(rhs.number) - (static_cast<int64_t>(rhs.number & 1 << B + A - 1) << 1);

        if (signed_lhs < 0) is_lhs_positive = -1;
        if (signed_rhs < 0) is_rhs_positive = -1;
        uint64_t lhs64 = std::abs(signed_lhs);
        uint64_t rhs64 = std::abs(signed_rhs);

        lhs64 <<= B + 1;

        uint64_t T = lhs64 / rhs64;

        uint32_t fractional_part = T & ((static_cast<uint64_t>(1) << B + 1) - 1);
        uint32_t full_part = T >> B + 1;
        bool is_further_bits = T * rhs64 != lhs64;

        bool is_negative = is_lhs_positive * is_rhs_positive == -1;

        BinaryRoundings binary_roundings(fractional_part, is_negative, B, is_further_bits, full_part);
        switch (rounding_type) {
            case RoundingTypes::TowardZero:
                fractional_part = binary_roundings.RoundTowardZero();
                break;
            case RoundingTypes::TowardInfinity:
                fractional_part = binary_roundings.RoundTowardInfinity();
                break;
            case RoundingTypes::TowardNegInfinity:
                fractional_part = binary_roundings.RoundTowardNegInfinity();
                break;
            case RoundingTypes::ToNearestEven:
                fractional_part = binary_roundings.RoundToNearestEven();
                break;
        }

        if (binary_roundings.overflow) {
            ++full_part;
            binary_roundings.overflow = false;
        }

        uint64_t bits = (full_part << B) + fractional_part;

        if (is_negative && bits) {
            bits = 0xffffffffffffffff - bits + 1;
        }

        uint32_t result = ModularArithmetic(bits, 64 - A - B);
        return PrintingNumber(A, B, result);
    }

    void Print() {
        PrintingNumber(A, B, number).Print(rounding_type);
    }

private:
    const uint8_t A;
    const uint8_t B;
    const uint32_t number;
    const RoundingTypes rounding_type;
};


enum class ObjectType {
    Infinity,
    NegInfinity,
    Nan,
    Denormal,
    Normal,
    Zero,
    NegZero
};

class FloatingPointNumber {
public:
    FloatingPointNumber(const std::string& format, uint32_t number, RoundingTypes r_type) {
        this->format = format;
        this->r_type = r_type;
        this->input = number;
        if (format == "f") {
            exp_bits_count = 8;
            mantis_bits_count = 23;
            number_bits_count = 32;
        }
        if (number_bits_count == 16) {
            number &= 0xffff;
        }

        sign = number & (1 << number_bits_count - 1);
        exp = (number & (1 << exp_bits_count) - 1 << mantis_bits_count) >> mantis_bits_count;
        real_exp = exp - ((1 << exp_bits_count - 1) - 1);
        mantis = number & (1 << mantis_bits_count) - 1;
        SetNumberType();

        if (object == ObjectType::Denormal) {
            int first_non_zero_bit_index = FindFirstNonZeroBit(mantis, mantis_bits_count);
            real_exp -= first_non_zero_bit_index;
            mantis <<= first_non_zero_bit_index + 1;
            mantis &= (1 << mantis_bits_count) - 1;
        }
    }

    void Print() const {
        using type = ObjectType;

        switch (object) {
            case type::Infinity:
                std::cout << "inf";
                break;
            case type::NegInfinity:
                std::cout << "-inf";
                break;
            case type::Nan:
                std::cout << "nan";
                break;
            case type::Zero:
                if (number_bits_count == 16) {
                    std::cout << "0x0.000p+0";
                } else {
                    std::cout << "0x0.000000p+0";
                }
                break;
            case type::NegZero:
                if (number_bits_count == 16) {
                    std::cout << "-0x0.000p+0";
                } else {
                    std::cout << "-0x0.000000p+0";
                }
                break;
            case type::Normal:
                PrintNormal();
                break;

            case type::Denormal:
                PrintDenormal();
                break;
        }

    }

    FloatingPointNumber operator*(const FloatingPointNumber& rhs) const {
        bool new_sign = sign ^ rhs.sign;
        {
            using type = ObjectType;
            if (object == type::Nan) {
                return FloatingPointNumber(format, input, r_type);
            }
            if (rhs.object == type::Nan) {
                return FloatingPointNumber(format, rhs.input, r_type);
            }
            if (((object == type::Zero) || (object == type::NegZero)) && ((rhs.object == type::Infinity) || (rhs.object == type::NegInfinity))) {
                return FloatingPointNumber(format, 0xffffffff, r_type);
            }
            if (((rhs.object == type::Zero) || (rhs.object == type::NegZero)) && ((object == type::Infinity) || (object == type::NegInfinity))) {
                return FloatingPointNumber(format, 0xffffffff, r_type);
            }
            if (((object == type::Zero) || (object == type::NegZero)) || ((rhs.object == type::Zero) || (rhs.object == type::NegZero))) {
                return GetZero(new_sign);
            }
            if (((object == type::Infinity) || (object == type::NegInfinity)) || ((rhs.object == type::Infinity) || (rhs.object == type::NegInfinity))) {
                return GetInfinity(new_sign);
            }
        }

        int new_exp = real_exp + rhs.real_exp;
        uint64_t new_mantis = static_cast<uint64_t>(mantis + (static_cast<uint64_t>(1) << mantis_bits_count)) * (rhs.mantis + (static_cast<uint64_t>(1) << mantis_bits_count));
        int new_mantis_bits_count = 64 - FindFirstNonZeroBit(new_mantis, 64);

        if (((format == "f") && (new_mantis_bits_count == 48)) || ((format == "h") && (new_mantis_bits_count == 22))) {
            ++new_exp;
        }

        if ((new_exp > 15 && format == "h") || (new_exp > 127 && format == "f")) {
            return Overflow(new_sign);
        }

        if ((new_exp < -24 && format == "h") || (new_exp < -149 && format == "f")) {
            return Underflow(new_sign, new_exp, ((mantis != 0) || (rhs.mantis != 0)));
        }

        bool got_overflow = false;

        if ((new_exp < -14 && format == "h") || (new_exp < -126 && format == "f")) {
            new_mantis = RoundDenormalMantis(new_mantis, new_sign, got_overflow, new_exp);
        } else {
            new_mantis = RoundMantis(new_mantis, new_sign, got_overflow);
        }

        if (got_overflow) {
            ++new_exp;
        }
        if ((new_exp > 15 && format == "h") || (new_exp > 127 && format == "f")) {
            return Overflow(new_sign);
        }

        if ((new_exp >= -14 && format == "h") || (new_exp >= -126 && format == "f")) {
            return MakeNumber(new_sign, new_exp + ((1 << exp_bits_count - 1) - 1), new_mantis);
        } else {
            return MakeNumber(new_sign, 0, new_mantis);
        }

    }

    FloatingPointNumber operator/(const FloatingPointNumber& rhs) const {
        bool new_sign = sign ^ rhs.sign;
        {
            using type = ObjectType;
            if (object == type::Nan) {
                return FloatingPointNumber(format, input, r_type);
            }
            if (rhs.object == type::Nan) {
                return FloatingPointNumber(format, rhs.input, r_type);
            }
            if (((object == type::Zero) || (object == type::NegZero)) && ((rhs.object == type::Infinity) || (rhs.object == type::NegInfinity))) {
                return GetZero(new_sign);
            }
            if (((rhs.object == type::Zero) || (rhs.object == type::NegZero)) && ((object == type::Infinity) || (object == type::NegInfinity))) {
                return GetInfinity(new_sign);
            }
            if (((object == type::Zero) || (object == type::NegZero)) && ((rhs.object == type::Zero) || (rhs.object == type::NegZero))) {
                return FloatingPointNumber(format, 0xffffffff, r_type);
            }
            if (((object == type::Infinity) || (object == type::NegInfinity)) && ((rhs.object == type::Infinity) || (rhs.object == type::NegInfinity))) {
                return FloatingPointNumber(format, 0xffffffff, r_type);
            }
            if (((object == type::Infinity) || (object == type::NegInfinity))) {
                return GetInfinity(new_sign);
            }
            if (((rhs.object == type::Infinity) || (rhs.object == type::NegInfinity))) {
                return GetZero(new_sign);
            }
            if (((object == type::Zero) || (object == type::NegZero))) {
                return GetZero(new_sign);
            }
            if (((rhs.object == type::Zero) || (rhs.object == type::NegZero))) {
                return GetInfinity(new_sign);
            }
        }

        int new_exp = real_exp - rhs.real_exp;

        if ((mantis + (static_cast<uint64_t>(1) << mantis_bits_count)) / (rhs.mantis + (static_cast<uint64_t>(1) << mantis_bits_count)) == 0) {
            --new_exp;
        }

        if ((new_exp > 15 && format == "h") || (new_exp > 127 && format == "f")) {
            return Overflow(new_sign);
        }
        if ((new_exp < -24 && format == "h") || (new_exp < -149 && format == "f")) {
            return Underflow(new_sign, new_exp, ((mantis != 0) || (rhs.mantis != 0)));
        }

        uint64_t new_mantis = (static_cast<uint64_t>(mantis + (static_cast<uint64_t>(1) << mantis_bits_count)) << mantis_bits_count + 1) / (rhs.mantis + (static_cast<uint64_t>(1) << mantis_bits_count));

        bool is_further_bits = (new_mantis * (rhs.mantis + (static_cast<uint64_t>(1) << mantis_bits_count))) !=
                ((static_cast<uint64_t>(mantis + (static_cast<uint64_t>(1) << mantis_bits_count))) << mantis_bits_count + 1);


        bool got_overflow = false;
        if ((new_exp < -14 && format == "h") || (new_exp < -126 && format == "f")) {
            new_mantis = RoundDenormalMantis(new_mantis, new_sign, got_overflow, new_exp, is_further_bits);
        } else {
            new_mantis = RoundMantis(new_mantis, new_sign, got_overflow, is_further_bits);
        }
        new_mantis &= (1 << mantis_bits_count) - 1;

        if (got_overflow) {
            ++new_exp;
        }
        if ((new_exp > 15 && format == "h") || (new_exp > 127 && format == "f")) {
            return Overflow(new_sign);
        }


        if ((new_exp >= -14 && format == "h") || (new_exp >= -126 && format == "f")) {
            return MakeNumber(new_sign, new_exp + ((1 << exp_bits_count - 1) - 1), new_mantis);
        } else {
            return MakeNumber(new_sign, 0, new_mantis);
        }
    }

    FloatingPointNumber operator+(const FloatingPointNumber& rhs) const {

        {
            using type = ObjectType;
            if (object == type::Nan) {
                return FloatingPointNumber(format, input, r_type);
            }
            if (rhs.object == type::Nan) {
                return FloatingPointNumber(format, rhs.input, r_type);
            }
            if ((object == type::NegInfinity) && (rhs.object == type::Infinity)) {
                return FloatingPointNumber(format, 0xffffffff, r_type);
            }
            if ((rhs.object == type::NegInfinity) && (object == type::Infinity)) {
                return FloatingPointNumber(format, 0xffffffff, r_type);
            }
            if ((rhs.object == type::NegInfinity) || (rhs.object == type::Infinity)) {
                return GetInfinity(rhs.sign);
            }
            if ((object == type::NegInfinity) || (object == type::Infinity)) {
                return GetInfinity(sign);
            }
            if ((rhs.exp == exp) && (rhs.mantis == mantis) && (sign ^ rhs.sign)) {
                if (r_type != RoundingTypes::TowardNegInfinity) {
                    return GetZero(false);
                }
                return GetZero(true);
            }
            if ((rhs.object == type::Zero) || (rhs.object == type::NegZero)) {
                return FloatingPointNumber(format, input, r_type);
            }
            if ((object == type::Zero) || (object == type::NegZero)) {
                return FloatingPointNumber(format, rhs.input, r_type);
            }
        }

        FloatingPointNumber big(format, 0, r_type);
        FloatingPointNumber small(format, 0, r_type);

        if (rhs.real_exp > real_exp) {
            big = rhs;
            small = *this;
        } else if (rhs.real_exp == real_exp) {
            if (rhs.mantis >= mantis) {
                big = rhs;
                small = *this;
            } else {
                big = *this;
                small = rhs;
            }
        } else {
            big = *this;
            small = rhs;
        }

        // числа одного знака
        if ((big.sign ^ small.sign) == 0) {
            int delta = std::min(big.real_exp - small.real_exp, 30);

            big.mantis += (static_cast<uint64_t>(1) << mantis_bits_count);
            small.mantis += (static_cast<uint64_t>(1) << mantis_bits_count);

            big.mantis <<= 1;
            small.mantis <<= 1;

            bool is_further_bits = small.mantis & ((1 << delta) - 1);
            small.mantis >>= delta;

            uint32_t res_mantis = big.mantis + small.mantis;
            int new_exp = big.real_exp;

            if (FindFirstNonZeroBit(res_mantis, 32) != FindFirstNonZeroBit(big.mantis, 32)) {
                ++new_exp;
            }

            if ((new_exp > 15 && format == "h") || (new_exp > 127 && format == "f")) {
                return Overflow(sign);
            }

            bool got_overflow = false;
            if ((new_exp < -14 && format == "h") || (new_exp < -126 && format == "f")) {
                res_mantis = RoundDenormalMantis(res_mantis, sign, got_overflow, new_exp, is_further_bits);
            } else {
                res_mantis = RoundMantis(res_mantis, sign, got_overflow, is_further_bits);
            }

            res_mantis &= (1 << mantis_bits_count) - 1;

            if (got_overflow) {
                ++new_exp;
            }
            if ((new_exp > 15 && format == "h") || (new_exp > 127 && format == "f")) {
                return Overflow(sign);
            }


            if ((new_exp >= -14 && format == "h") || (new_exp >= -126 && format == "f")) {
                return MakeNumber(sign, new_exp + ((1 << exp_bits_count - 1) - 1), res_mantis);
            } else {
                return MakeNumber(sign, 0, res_mantis);
            }
        }

        // числа разного знака

        bool new_sign = big.sign;
        int delta = big.real_exp - small.real_exp;
        int new_exp = big.real_exp;

        // поправил, что дельта может переполниться и для format = "h"
        if (delta < 33) {
            uint64_t big_mantis = big.mantis + (static_cast<uint64_t>(1) << mantis_bits_count);
            uint64_t small_mantis = small.mantis + (static_cast<uint64_t>(1) << mantis_bits_count);

            big_mantis <<= 35;
            small_mantis <<= 35;
            small_mantis >>= delta;

            int f1 = FindFirstNonZeroBit(big_mantis, 64);
            uint64_t res_mantis = big_mantis - small_mantis;
            int f2 = FindFirstNonZeroBit(res_mantis, 64);

            new_exp -= (std::abs(f2 - f1));

            if ((new_exp < -24 && format == "h") || (new_exp < -149 && format == "f")) {
                return Underflow(new_sign, new_exp, ((mantis != 0) || (rhs.mantis != 0)));
            }

            bool got_overflow = false;
            if ((new_exp < -14 && format == "h") || (new_exp < -126 && format == "f")) {
                res_mantis = RoundDenormalMantis(res_mantis, new_sign, got_overflow, new_exp, false);
            } else {
                res_mantis = RoundMantis(res_mantis, new_sign, got_overflow, false);
            }

            res_mantis &= (1 << mantis_bits_count) - 1;

            if (got_overflow) {
                ++new_exp;
            }
            if ((new_exp > 15 && format == "h") || (new_exp > 127 && format == "f")) {
                return Overflow(new_sign);
            }

            if ((new_exp >= -14 && format == "h") || (new_exp >= -126 && format == "f")) {
                return MakeNumber(new_sign, new_exp + ((1 << exp_bits_count - 1) - 1), res_mantis);
            } else {
                return MakeNumber(new_sign, 0, res_mantis);
            }
        }


        using type = RoundingTypes;

        if (new_sign) {
            switch (r_type) {
                case type::TowardZero:
                    if (big.mantis == 0) {
                        --big.real_exp;
                        if ((big.real_exp > 15 && format == "h") || (big.real_exp > 127 && format == "f")) {
                            return Overflow(new_sign);
                        }
                        big.mantis = (1 << mantis_bits_count) - 1;
                    } else {
                        --big.mantis;
                    }
                    return big;
                case type::TowardNegInfinity:
                    return big;
                case type::TowardInfinity:
                    if (big.mantis == 0) {
                        --big.real_exp;
                        if ((big.real_exp > 15 && format == "h") || (big.real_exp > 127 && format == "f")) {
                            return Overflow(new_sign);
                        }
                        big.mantis = (1 << mantis_bits_count) - 1;
                    } else {
                        --big.mantis;
                    }
                    return big;
                case type::ToNearestEven:
                    return big;
            }
        }

        switch (r_type) {
            case type::TowardZero:
                if (big.mantis == 0) {
                    --big.real_exp;
                    if ((big.real_exp > 15 && format == "h") || (big.real_exp > 127 && format == "f")) {
                        return Overflow(new_sign);
                    }
                    big.mantis = (1 << mantis_bits_count) - 1;
                } else {
                    --big.mantis;
                }
                return big;
            case type::TowardNegInfinity:
                if (big.mantis == 0) {
                    --big.real_exp;
                    if ((big.real_exp > 15 && format == "h") || (big.real_exp > 127 && format == "f")) {
                        return Overflow(new_sign);
                    }
                    big.mantis = (1 << mantis_bits_count) - 1;
                } else {
                    --big.mantis;
                }
                return big;
            case type::TowardInfinity:
                return big;
            case type::ToNearestEven:
                return big;
        }

    }

    FloatingPointNumber operator-(const FloatingPointNumber& rhs) const {
        return *this + FloatingPointNumber(rhs.format, (rhs.input ^ (1 << rhs.number_bits_count - 1)), rhs.r_type);
    }

private:
    void SetNumberType() {
        if (exp == 0) {
            if (mantis == 0) {
                if (sign) {
                    object = ObjectType::NegZero;
                } else {
                    object = ObjectType::Zero;
                }
            } else {
                object = ObjectType::Denormal;
            }
        } else if (exp == (1 << exp_bits_count) - 1) {
            if (mantis == 0) {
                if (sign) {
                    object = ObjectType::NegInfinity;
                } else {
                    object = ObjectType::Infinity;
                }
            } else {
                object = ObjectType::Nan;
            }
        } else {
            object = ObjectType::Normal;
        }
    }

    void PrintNormal() const {
        if (sign) {
            std::cout << "-";
        }
        std::cout << "0x1.";

        long long mantis_ = mantis;

        if (mantis_bits_count == 10) {
            printf("%03llx", mantis_ << 2);
        } else {
            printf("%06llx", mantis_ << 1);
        }

        std::cout << "p";
        if (real_exp >= 0) {
            std::cout << "+";
        }
        std::cout << real_exp;
    }

    void PrintDenormal() const {
        PrintNormal();
    }

    template<typename T>
    int FindFirstNonZeroBit(T bits, uint16_t significant_bits_count) const {
        if (significant_bits_count != 64) {
            bits &= (static_cast<uint64_t>(1) << significant_bits_count) - 1;
        }

        if (!bits) {return -1;}

        int index = 0;
        while (!((static_cast<uint64_t>(1) << significant_bits_count - 1 - index) & bits)) ++index;

        return index;
    }

    FloatingPointNumber GetMaximumNumber(bool sign) const {
        return MakeNumber(sign, (1 << exp_bits_count) - 2, (1 << mantis_bits_count) - 1);
    }

    FloatingPointNumber GetMinimumNumber(bool sign) const {
        return MakeNumber(sign, 0, 1);
    }

    FloatingPointNumber GetInfinity(bool sign) const {
        return MakeNumber(sign, (1 << exp_bits_count) - 1, 0);
    }

    FloatingPointNumber GetZero(bool sign) const {
        return MakeNumber(sign, 0, 0);
    }

    FloatingPointNumber Overflow(bool sign) const {
        using type = RoundingTypes;
        switch (r_type) {
            case type::TowardZero:
                return GetMaximumNumber(sign);
            case type::ToNearestEven:
                return GetInfinity(sign);
            case type::TowardInfinity:
                if (sign) {
                    return GetMaximumNumber(sign);
                }
                return GetInfinity(sign);
            case type::TowardNegInfinity:
                if (sign) {
                    return GetInfinity(sign);
                }
                return GetMaximumNumber(sign);
        }
    }

    FloatingPointNumber Underflow(bool sign, int real_exp, bool non_zero_mantis) const {
        using type = RoundingTypes;
        switch (r_type) {
            case type::TowardZero:
                return GetZero(sign);
            case type::ToNearestEven:
                if ((real_exp < -150 && format == "f") || (real_exp < -25 && format == "h") || (!non_zero_mantis)) {
                    return GetZero(sign);
                }
                return GetMinimumNumber(sign);
            case type::TowardInfinity:
                if (sign) {
                    return GetZero(sign);
                }
                return GetMinimumNumber(sign);
            case type::TowardNegInfinity:
                if (sign) {
                    return GetMinimumNumber(sign);
                }
                return GetZero(sign);
        }
    }

    FloatingPointNumber MakeNumber(bool sign, uint32_t exp, uint32_t mantis) const {
        uint32_t new_number = 0;
        new_number += sign;
        new_number <<= exp_bits_count;
        new_number += exp;
        new_number <<= mantis_bits_count;
        new_number += mantis;
        return FloatingPointNumber(format, new_number, r_type);
    }

    uint32_t RoundMantis(uint64_t mantis, bool sign, bool& get_overflow, bool is_further_bits = false) const {
        int first_non_zero_bit_index = FindFirstNonZeroBit(mantis, 64);

        int to_cut_bits_count = 64 - first_non_zero_bit_index - mantis_bits_count - 1;

        bool last_bit = false;
        bool further = false;

        if (to_cut_bits_count <= 0) {
            further = is_further_bits;
        } else {
            last_bit = mantis & (static_cast<uint64_t>(1) << to_cut_bits_count - 1);
            if (to_cut_bits_count != 1) {
                further = mantis & (static_cast<uint64_t>(1) << to_cut_bits_count - 1) - 1;
            }
        }

        if (is_further_bits) further = true;

        uint32_t real_mantis = (mantis >> to_cut_bits_count) & (static_cast<uint64_t>(1) << mantis_bits_count) - 1;


        switch (r_type) {
            case RoundingTypes::TowardZero:
                return real_mantis;
            case RoundingTypes::TowardInfinity:
                if (sign) {
                    return real_mantis;
                }
                if (last_bit || further) {
                    ++real_mantis;
                    if ((real_mantis & (1 << mantis_bits_count) - 1) == 0) {
                        get_overflow = true;
                    }
                }
                break;
            case RoundingTypes::TowardNegInfinity:
                if (!sign) {
                    return real_mantis;
                }
                if (last_bit || further) {
                    ++real_mantis;
                    if ((real_mantis & (1 << mantis_bits_count) - 1) == 0) {
                        get_overflow = true;
                    }
                }
                break;
            case RoundingTypes::ToNearestEven:
                if (!last_bit) {
                    return real_mantis;
                }
                if (further) {
                    ++real_mantis;
                    if ((real_mantis & (1 << mantis_bits_count) - 1) == 0) {
                        get_overflow = true;
                    }
                }
                else {
                    if (real_mantis % 2 == 0) {
                        return real_mantis;
                    } else {
                        ++real_mantis;
                        if ((real_mantis & (1 << mantis_bits_count) - 1) == 0) {
                            get_overflow = true;
                        }
                    }
                }
                break;
        }


        real_mantis &= (1 << mantis_bits_count) - 1;


        return real_mantis;
    }

    uint32_t RoundDenormalMantis(uint64_t mantis, bool sign, bool& get_overflow, int new_exp, bool is_further_bits = false) const {
        int first_non_zero_bit_index = FindFirstNonZeroBit(mantis, 64);
        int mantis_start_index;

        if (format == "f") {
            mantis_start_index = first_non_zero_bit_index + (126 + new_exp) + 1;
        } else {
            mantis_start_index = first_non_zero_bit_index + (14 + new_exp) + 1;
        }

        int to_cut_bits_count = 64 - mantis_start_index - mantis_bits_count;

        bool last_bit = false;
        bool further = false;

        if (to_cut_bits_count <= 0) {
            further = is_further_bits;
        } else {
            last_bit = mantis & (static_cast<uint64_t>(1) << to_cut_bits_count - 1);
            if (to_cut_bits_count != 1) {
                further = mantis & (static_cast<uint64_t>(1) << to_cut_bits_count) - 1;
            }
        }

        if (is_further_bits) further = true;

        uint32_t real_mantis = (mantis >> to_cut_bits_count) & (static_cast<uint64_t>(1) << mantis_bits_count) - 1;

        switch (r_type) {
            case RoundingTypes::TowardZero:
                return real_mantis;
            case RoundingTypes::TowardInfinity:
                if (sign) {
                    return real_mantis;
                }
                if (last_bit || further) {
                    ++real_mantis;
                    if ((real_mantis & (1 << mantis_bits_count) - 1) == 0) {
                        get_overflow = true;
                    }
                }
                break;
            case RoundingTypes::TowardNegInfinity:
                if (!sign) {
                    return real_mantis;
                }
                if (last_bit || further) {
                    ++real_mantis;
                    if ((real_mantis & (1 << mantis_bits_count) - 1) == 0) {
                        get_overflow = true;
                    }
                }
                break;
            case RoundingTypes::ToNearestEven:
                if (!last_bit) {
                    return real_mantis;
                }
                if (further) {
                    ++real_mantis;
                    if ((real_mantis & (1 << mantis_bits_count) - 1) == 0) {
                        get_overflow = true;
                    }
                }
                else {
                    if (real_mantis % 2 == 0) {
                        return real_mantis;
                    } else {
                        ++real_mantis;
                        if ((real_mantis & (1 << mantis_bits_count) - 1) == 0) {
                            get_overflow = true;
                        }
                    }
                }
                break;
        }

        real_mantis &= (1 << mantis_bits_count) - 1;

        return real_mantis;
    }

    bool sign;
    uint8_t exp;
    int32_t real_exp;
    uint32_t mantis;
    uint8_t exp_bits_count = 5;
    uint8_t mantis_bits_count = 10;
    uint8_t number_bits_count = 16;
    uint32_t input;
    std::string format;
    ObjectType object;
    RoundingTypes r_type;
};



struct Options {
    bool fixed_point = false;
    bool floating_point = false;
    std::string floating_type;
    int A;
    int B;
    uint32_t a;
    std::string operation;
    uint32_t b;
    RoundingTypes r_type;

    bool IsValid() const {
        if (fixed_point) {
            if (A <= 0 || B < 0 || A + B > 32) {
                std::cerr << "invalid format";
                return false;
            }
            return true;
        }
        return true;
    }
};

namespace parsing {
    RoundingTypes GetRoundingTypeByString(std::string& round_type) {
        RoundingTypes r_type;
        if (round_type == "0") {
            r_type = RoundingTypes::TowardZero;
        } else if (round_type == "1") {
            r_type = RoundingTypes::ToNearestEven;
        } else if (round_type == "2") {
            r_type = RoundingTypes::TowardInfinity;
        } else if (round_type == "3") {
            r_type = RoundingTypes::TowardNegInfinity;
        } else {
            std::cerr << "Invalid round_type";
            exit(1);
        }
        return r_type;
    }

    int GetDigitFromChar(char digit_char) {
        static const int kNineCode = 57;
        static const int kZeroCode = 48;
        static const int kaCode = 97;
        static const int kfCode = 102;
        static const int kACode = 65;
        static const int kFCode = 70;

        int char_code = static_cast<unsigned char>(digit_char);

        if (char_code >= kZeroCode && char_code <= kNineCode) {
            return char_code - kZeroCode;
        } if (char_code >= kaCode && char_code <= kfCode) {
            return char_code - kaCode + 10;
        } if (char_code >= kACode && char_code <= kFCode) {
            return char_code - kACode + 10;
        }

        std::cerr << "not a digit";
        exit(1);
    }

    uint32_t ParseNumber(std::string& string_number) {
        if (string_number.size() < 3) {
            std::cerr << "invalid number";
            exit(1);
        }
        if (string_number[0] != '0' || string_number[1] != 'x') {
            std::cerr << "invalid number";
            exit(1);
        }
        int digits_count = string_number.size() - 2;
        int start_index = 2 + std::max(0, (digits_count - 8));
        uint32_t number = 0;
        for (int i = start_index; i < string_number.size(); ++i) {
            number += GetDigitFromChar(string_number[i]);
            if (i != string_number.size() - 1) {
                number <<= 4;
            }
        }

        return number;
    }

    std::pair<int,int> ParseFixedPointFormat(std::string& format) {
        if (format.find('.') == std::string::npos) {
            std::cerr << "invalid format";
            exit(1);
        }

        int point_index = format.find('.');
        if (point_index > 2 || point_index == 0) {
            std::cerr << "invalid format";
            exit(1);
        }

        static const int kNineCode = 57;
        static const int kZeroCode = 48;


        int A, B;
        if (point_index == 1) {
            int char_code = static_cast<unsigned char>(format[0]);
            if (char_code >= kZeroCode && char_code <= kNineCode) {
                A = char_code - kZeroCode;
            } else {
                std::cerr << "invalid format";
                exit(1);
            }
        } else if (point_index == 2) {
            int char_code = static_cast<unsigned char>(format[0]);
            if (char_code >= kZeroCode && char_code <= kNineCode) {
                A = (char_code - kZeroCode) * 10;
            } else {
                std::cerr << "invalid format";
                exit(1);
            }
            char_code = format[1];
            if (char_code >= kZeroCode && char_code <= kNineCode) {
                A += char_code - kZeroCode;
            } else {
                std::cerr << "invalid format";
                exit(1);
            }
        }

        if (format.size() - point_index - 1 <= 0 || format.size() - point_index - 1 > 2) {
            std::cerr << "invalid format";
            exit(1);
        }

        if (format.size() - point_index - 1 == 1) {
            int char_code = static_cast<unsigned char>(format[point_index + 1]);
            if (char_code >= kZeroCode && char_code <= kNineCode) {
                B = char_code - kZeroCode;
            } else {
                std::cerr << "invalid format";
                exit(1);
            }
        } else if (format.size() - point_index - 1  == 2) {
            int char_code = static_cast<unsigned char>(format[point_index + 1]);
            if (char_code >= kZeroCode && char_code <= kNineCode) {
                B = (char_code - kZeroCode) * 10;
            } else {
                std::cerr << "invalid format";
                exit(1);
            }
            char_code = format[point_index + 2];
            if (char_code >= kZeroCode && char_code <= kNineCode) {
                B += char_code - kZeroCode;
            } else {
                std::cerr << "invalid format";
                exit(1);
            }
        }

        return std::make_pair(A, B);
    }

    std::string ParseOperation(std::string& operation) {
        if (operation != "+" && operation != "-" && operation != "M" && operation != "/") {
            std::cerr << "invalid operation";
            exit(1);
        }
        return operation;
    }

    Options Parse(int argc, char** argv) {
        Options options;
        if (argc == 4) {
            std::string format = argv[1];
            std::string round_type = argv[2];
            std::string number = argv[3];

            if (format == "f" || format == "h") {
                options.floating_point = true;
                options.floating_type = format;
                options.r_type = GetRoundingTypeByString(round_type);
                options.a = ParseNumber(number);
            } else {
                options.fixed_point = true;
                options.A = ParseFixedPointFormat(format).first;
                options.B = ParseFixedPointFormat(format).second;
                options.r_type = GetRoundingTypeByString(round_type);
                options.a = ParseNumber(number);
            }

        } else if (argc == 6) {
            std::string format = argv[1];
            std::string round_type = argv[2];
            std::string number1 = argv[3];
            std::string operation = argv[4];
            std::string number2 = argv[5];

            if (format == "f" || format == "h") {
                options.floating_point = true;
                options.floating_type = format;
                options.r_type = GetRoundingTypeByString(round_type);
                options.a = ParseNumber(number1);
                options.b = ParseNumber(number2);
                options.operation = ParseOperation(operation);
            } else {
                options.fixed_point = true;
                options.A = ParseFixedPointFormat(format).first;
                options.B = ParseFixedPointFormat(format).second;
                options.r_type = GetRoundingTypeByString(round_type);
                options.a = ParseNumber(number1);
                options.b = ParseNumber(number2);
                options.operation = ParseOperation(operation);
            }

        } else {
            std::cerr << "invalid count of args";
            exit(1);
        }
        return options;
    }
}

uint32_t CutUselessBits(uint32_t number, int A, int B) {
    return number << 32 - A - B >> 32 - A - B;
}

int main(int argc, char** argv) {
    Options options = parsing::Parse(argc, argv);
    if (!options.IsValid()) {
        exit(1);
    }

    RoundingTypes r_type = options.r_type;
    uint32_t a = options.a;

    if (options.fixed_point) {
        int A = options.A;
        int B = options.B;
        a = CutUselessBits(a, A, B);
        if (options.operation.empty()) {
            FixedPointNumber(A, B, a, r_type).Print();
        } else {
            uint32_t b = options.b;
            b = CutUselessBits(b, A, B);
            if (options.operation == "+") {
                (FixedPointNumber(A, B, a, r_type) + FixedPointNumber(A, B, b, r_type)).Print(r_type);
            } else if (options.operation == "-") {
                (FixedPointNumber(A, B, a, r_type) - FixedPointNumber(A, B, b, r_type)).Print(r_type);
            } else if (options.operation == "M") {
                (FixedPointNumber(A, B, a, r_type) * FixedPointNumber(A, B, b, r_type)).Print(r_type);
            } else if (options.operation == "/") {
                (FixedPointNumber(A, B, a, r_type) / FixedPointNumber(A, B, b, r_type)).Print(r_type);
            }
        }
    } else {
        std::string floating_type = options.floating_type;
        FloatingPointNumber x(options.floating_type, a, r_type);

        if (options.operation.empty()) {
            x.Print();
        } else {
            uint32_t b = options.b;
            FloatingPointNumber y(options.floating_type, b, r_type);
            if (options.operation == "+") {
                (x + y).Print();
            } else if (options.operation == "-") {
                (x - y).Print();
            } else if (options.operation == "M") {
                (x * y).Print();
            } else if (options.operation == "/") {
                (x / y).Print();
            }
        }
    }
}