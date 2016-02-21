/*
 * CS:APP Data Lab 
 * 
 * <Name: Yuanyuan Ma User ID: yuanyuam@andrew.cmu.edu>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * thirdBits - return word with every third bit (starting from the LSB) set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 1
 */
int thirdBits(void) {
    /
    int x = ((0x49 << 9) | 0x49);
    //printf("--%d\n",x);
    //printf("--%d\n",(x<<16) | x);
    return (x<<18) | x;
}
/*
 * isTmin - returns 1 if x is the minimum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmin(int x) {
    //int isZero = !x;
    return !(x + x) & !!(x);
}
//2
/* 
 * isNotEqual - return 0 if x == y, and 1 otherwise 
 *   Examples: isNotEqual(5,5) = 0, isNotEqual(4,5) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int isNotEqual(int x, int y) {
    return !!(x^y);
}
/* 
 * anyOddBit - return 1 if any odd-numbered bit in word set to 1
 *   Examples anyOddBit(0x5) = 0, anyOddBit(0x7) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int anyOddBit(int x) {
    int mask = 0xaa | (0xaa<<8);
    return !!(x & (mask | (mask << 16)));
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return (~x)+1;
}
//3
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
    int flag = !x;
    return ((~!flag+1) & y) | ((~flag+1) & z);
}
/* 
 * subOK - Determine if can compute x-y without overflow
 *   Example: subOK(0x80000000,0x80000000) = 1,
 *            subOK(0x80000000,0x70000000) = 0, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int subOK(int x, int y) {
    int signs = (x + (~y + 1)) >> 31;
    return !(((x >> 31)^(y >> 31)) & ((x >> 31) ^ signs));
  
}
/* 
 * isGreater - if x > y  then return 1, else return 0 
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
    int signx = x>>31;
    int signy = y>>31;
    return ((!signx) & signy) | ( (!(signx^signy)) & !((x+~y)>>31) );
}
//4
/*
 * bitParity - returns 1 if x contains an odd number of 0's
 *   Examples: bitParity(5) = 0, bitParity(7) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 4
 */
int bitParity(int x) {
    x ^= x >> 16;
    x ^= x >> 8;
    x ^= x >> 4;
    x ^= x >> 2;
    x ^= x >> 1;
    return x & 1;
    
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
	int sign, bits, b16, b8, b4, b2, b1;
    // make the number positive
    sign = x >> 31;
    x ^= sign;
    
    bits = 0;
    b16 = !!(x >> 16) << 4;
    bits += b16;
    b8  = !!(x >> (bits + 8)) <<3;
    bits += b8;
    b4 = !!(x >> (bits + 4)) << 2;
    bits += b4;
    b2 = !!(x >> (bits+ 2)) << 1;
    bits += b2;
    b1 = !!(x >>(bits+1));
    bits += b1 + 2;
    return (!x) | (bits & (~(!!x)+1));
  
}
//float
/*
 * float_half - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_half(unsigned uf) {
	int sign, exp, frac, carry;

    sign = uf & 0x80000000;
    exp = uf & 0x7F800000;
    frac = uf & 0x007FFFFF;

    // Special values : infinity or NaN
    if (exp == 0x7F800000) {
        return uf;
    }
    
    carry = 0;
    if ((uf & 3) == 3) {
        carry = 1;
    }
    
    /**
     * Denormalized values
     * E = 1 - bias; M = frac
     */
    if (exp == 0x00000000) {
        return sign | ((frac >> 1) + carry);
    }
    
    /**
     * Special case: exp = 1
     * In this case, normalized values -> denormalized values
     */
    if (exp == 0x00800000){
        return sign | (((uf & 0x7FFFFFFF)>>1)+ carry) ;
    }
    
    /**
     * Normalized values
     * E = exp - bias; M = 1 + frac
     */
    return (((exp >> 23) -1)<<23) | (uf &  0x807FFFFF);

}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
	unsigned sign, frac, E, exp, result;
    if (x == 0)  return 0;
    
    sign = x & 0x80000000;
    
    // frac is equal to the absolute value of x at the beginning
    frac = x;
    if (x < 0) {
        frac = -x;
    }
    
    // calculate the exp
    E = 0;
    while(!(frac & 0x80000000)){
        frac <<= 1;
        E ++;
    }
    
    frac <<= 1;
    E++;
    exp = (159 - E) << 23;
    
    result = sign | exp | (frac >> 9);
    // rounding
    if (((frac & 0x000001ff) > 0x00000100) || ((frac & 0x000003ff) == 0x00000300 )){
        return result + 1;
    }else{
        return result;
    }
    
}
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
	int sign, exp, frac, E, M;
    
    sign = uf & 0x80000000;
    exp = uf & 0x7F800000;
    frac = uf & 0x007FFFFF;
    
    // Special values : infinity or NaN
    if (exp == 0x7F800000) {
        return 0x80000000u;
    }
    
    // Denormalized values
    if (exp == 0x00000000) {
        return 0;
    }

    
    E = (exp >> 23) - 127;
    // Special cases
    if (E > 30) {
        return 0x80000000u;
    }
    
    if (E < 0) {
        return 0;
    }
    
    M = frac | 0x800000;
    if (E > 23) {
        M <<= (E - 23);
    }else{
        M >>= (23 - E);
    }
    
    if (sign == 0x80000000) {
        return ~M + 1;
    }else{
        return M;
    }
    
}
