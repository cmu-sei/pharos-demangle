// Pharos Demangler
//
// Copyright 2017 Carnegie Mellon University. All Rights Reserved.
//
// NO WARRANTY. THIS CARNEGIE MELLON UNIVERSITY AND SOFTWARE ENGINEERING
// INSTITUTE MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. CARNEGIE MELLON
// UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR
// IMPLIED, AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF
// FITNESS FOR PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS
// OBTAINED FROM USE OF THE MATERIAL. CARNEGIE MELLON UNIVERSITY DOES NOT
// MAKE ANY WARRANTY OF ANY KIND WITH RESPECT TO FREEDOM FROM PATENT,
// TRADEMARK, OR COPYRIGHT INFRINGEMENT.
//
// Released under a BSD-style license, please see license.txt or contact
// permission@sei.cmu.edu for full terms.
//
// [DISTRIBUTION STATEMENT A] This material has been approved for public
// release and unlimited distribution.  Please see Copyright notice for
// non-US Government use and distribution.
//
// DM17-0949

// This file is meant to be included after setting CODE_ENUM(enum_symbol,
// enum_string) to an appropriate macro.  The CODE_ENUM macro will be
// undefined at the end of this file.

#ifndef CODE_ENUM
#  error "CODE_ENUM() has not been defined"
#endif

CODE_ENUM(UNDEFINED,                         ""),

CODE_ENUM(BOOL,                              "bool"),
CODE_ENUM(SIGNED_CHAR,                       "signed char"),
CODE_ENUM(CHAR,                              "char"),
CODE_ENUM(UNSIGNED_CHAR,                     "unsigned char"),
CODE_ENUM(SHORT,                             "short"),
CODE_ENUM(UNSIGNED_SHORT,                    "unsigned short"),
CODE_ENUM(INT,                               "int"),
CODE_ENUM(UNSIGNED_INT,                      "unsigned int"),
CODE_ENUM(LONG,                              "long"),
CODE_ENUM(UNSIGNED_LONG,                     "unsigned long"),
CODE_ENUM(FLOAT,                             "float"),
CODE_ENUM(DOUBLE,                            "double"),
CODE_ENUM(LONG_DOUBLE,                       "long double"),

CODE_ENUM(INT8,                              "int8_t"),
CODE_ENUM(UINT8,                             "uint8_t"),
CODE_ENUM(INT16,                             "int16_t"),
CODE_ENUM(UINT16,                            "uint16_t"),
CODE_ENUM(INT32,                             "int32_t"),
CODE_ENUM(UINT32,                            "uint32_t"),
CODE_ENUM(INT64,                             "int64_t"),
CODE_ENUM(UINT64,                            "uint64_t"),
CODE_ENUM(CHAR16,                            "char16_t"),
CODE_ENUM(CHAR32,                            "char32_t"),
CODE_ENUM(WCHAR,                             "wchar_t"),

CODE_ENUM(INT128,                            "__int128"),
CODE_ENUM(UINT128,                           "unsigned __int128"),

CODE_ENUM(VOID,                              "void"),
CODE_ENUM(ELLIPSIS,                          "..."),

CODE_ENUM(UNION,                             "union"),
CODE_ENUM(CLASS,                             "class"),
CODE_ENUM(STRUCT,                            "struct"),
CODE_ENUM(ENUM,                              "enum"),

CODE_ENUM(CTOR,                              "`constructor'"),
CODE_ENUM(DTOR,                              "`destructor'"),
CODE_ENUM(OP_NEW,                            "operator new"),
CODE_ENUM(OP_DELETE,                         "operator delete"),
CODE_ENUM(OP_ASSIGN,                         "operator="),
CODE_ENUM(OP_RSHIFT,                         "operator>>"),
CODE_ENUM(OP_LSHIFT,                         "operator<<"),
CODE_ENUM(OP_NOT,                            "operator!"),
CODE_ENUM(OP_EQUAL,                          "operator=="),
CODE_ENUM(OP_NOTEQUAL,                       "operator!="),
CODE_ENUM(OP_INDEX,                          "operator[]"),
CODE_ENUM(OP_TYPE,                           "operator `type'"),
CODE_ENUM(OP_INDIRECT,                       "operator->"),
CODE_ENUM(OP_STAR,                           "operator*"),
CODE_ENUM(OP_PLUSPLUS,                       "operator++"),
CODE_ENUM(OP_MINUSMINUS,                     "operator--"),
CODE_ENUM(OP_MINUS,                          "operator-"),
CODE_ENUM(OP_PLUS,                           "operator+"),
CODE_ENUM(OP_AMP,                            "operator&"),
CODE_ENUM(OP_INDIRECT_METHOD,                "operator->*"),
CODE_ENUM(OP_DIV,                            "operator/"),
CODE_ENUM(OP_MOD,                            "operator%"),
CODE_ENUM(OP_LESS,                           "operator<"),
CODE_ENUM(OP_LESSEQ,                         "operator<="),
CODE_ENUM(OP_GREATER,                        "operator>"),
CODE_ENUM(OP_GREATEREQ,                      "operator>="),
CODE_ENUM(OP_COMMA,                          "operator,"),
CODE_ENUM(OP_CALL,                           "operator()"),
CODE_ENUM(OP_BNOT,                           "operator~"),
CODE_ENUM(OP_BXOR,                           "operator^"),
CODE_ENUM(OP_BOR,                            "operator|"),
CODE_ENUM(OP_AND,                            "operator&&"),
CODE_ENUM(OP_OR,                             "operator||"),
CODE_ENUM(OP_STAR_ASSIGN,                    "operator*="),
CODE_ENUM(OP_PLUS_ASSIGN,                    "operator+="),
CODE_ENUM(OP_MINUS_ASSIGN,                   "operator-="),
CODE_ENUM(OP_DIV_ASSIGN,                     "operator/="),
CODE_ENUM(OP_MOD_ASSIGN,                     "operator%="),
CODE_ENUM(OP_RSHIFT_ASSIGN,                  "operator>>="),
CODE_ENUM(OP_LSHIFT_ASSIGN,                  "operator<<="),
CODE_ENUM(OP_AMP_ASSIGN,                     "operator&="),
CODE_ENUM(OP_BOR_ASSIGN,                     "operator|="),
CODE_ENUM(OP_BXOR_ASSIGN,                    "operator^="),
CODE_ENUM(VFTABLE,                           "`vftable'"),
CODE_ENUM(VBTABLE,                           "`vbtable'"),
CODE_ENUM(VCALL,                             "`vcall'"),
CODE_ENUM(TYPEOF,                            "`typeof'"),
CODE_ENUM(LOCAL_STATIC_GUARD,                "`local static guard'"),
CODE_ENUM(VBASE_DTOR,                        "`vbase destructor'"),
CODE_ENUM(VECTOR_DELETING_DTOR,              "`vector deleting destructor'"),
CODE_ENUM(DEFAULT_CTOR_CLOSURE,              "`default constructor closure'"),
CODE_ENUM(SCALAR_DELETING_DTOR,              "`scalar deleting destructor'"),
CODE_ENUM(VECTOR_CTOR_ITER,                  "`vector constructor iterator'"),
CODE_ENUM(VECTOR_DTOR_ITER,                  "`vector destructor iterator'"),
CODE_ENUM(VECTOR_VBASE_CTOR_ITER,            "`vector vbase constructor iterator'"),
CODE_ENUM(VIRTUAL_DISPLACEMENT_MAP,          "`virtual displacement map'"),
CODE_ENUM(EH_VECTOR_CTOR_ITER,               "`eh vector constructor iterator'"),
CODE_ENUM(EH_VECTOR_DTOR_ITER,               "`eh vector destructor iterator'"),
CODE_ENUM(EH_VECTOR_VBASE_CTOR_ITER,         "`eh vector vbase constructor iterator'"),
CODE_ENUM(COPY_CTOR_CLOSURE,                 "`copy constructor closure'"),
CODE_ENUM(UDT_RETURNING,                     "`udt returning'"),
CODE_ENUM(LOCAL_VFTABLE,                     "`local vftable'"),
CODE_ENUM(LOCAL_VFTABLE_CTOR_CLOSURE,        "`local vftable constructor closure'"),
CODE_ENUM(OP_NEW_ARRAY,                      "operator new[]"),
CODE_ENUM(OP_DELETE_ARRAY,                   "operator delete[]"),
CODE_ENUM(PLACEMENT_DELETE_CLOSURE,          "`placement delete closure'"),
CODE_ENUM(PLACEMENT_DELETE_ARRAY_CLOSURE,    "`placement delete[] closure'"),
CODE_ENUM(MANAGED_VECTOR_CTOR_ITER,          "`managed vector constructor iterator'"),
CODE_ENUM(MANAGED_VECTOR_DTOR_ITER,          "`managed vector destructor iterator'"),
CODE_ENUM(EH_VECTOR_COPY_CTOR_ITER,          "`eh vector copy constructor iterator'"),
CODE_ENUM(EH_VECTOR_VBASE_COPY_CTOR_ITER,    "`eh vector vbase copy constructor iterator'"),
CODE_ENUM(DYNAMIC_INITIALIZER,               "`dynamic initializer'"),
CODE_ENUM(DYNAMIC_ATEXIT_DTOR,               "`dynamic atexit destructor'"),
CODE_ENUM(VECTOR_COPY_CTOR_ITER,             "`vector copy constructor iterator'"),
CODE_ENUM(VECTOR_VBASE_COPY_CTOR_ITER,       "`vector vbase copy constructor iterator'"),
CODE_ENUM(MANAGED_VECTOR_COPY_CTOR_ITER,     "`managed vector copy constructor iterator'"),
CODE_ENUM(LOCAL_STATIC_THREAD_GUARD,         "`local static thread guard'"),
CODE_ENUM(OP_DQUOTE,                         "operator\"\""),

CODE_ENUM(RTTI_TYPE_DESC,                    "`RTTI Type Descriptor'"),
CODE_ENUM(RTTI_BASE_CLASS_DESC,              "`RTTI Base Class Descriptor'"),
CODE_ENUM(RTTI_BASE_CLASS_ARRAY,             "`RTTI Base Class Array'"),
CODE_ENUM(RTTI_CLASS_HEIRARCHY_DESC,         "`RTTI Class Hierarchy Descriptor'"),
CODE_ENUM(RTTI_COMPLETE_OBJ_LOCATOR,         "`RTTI Complete Object Locator'")

#undef CODE_ENUM
