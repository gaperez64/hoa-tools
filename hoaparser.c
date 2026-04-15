/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "hoa.y"

/**************************************************************************
 * Copyright (c) 2019- Guillermo A. Perez
 * 
 * This file is part of HOATOOLS.
 * 
 * HOATOOLS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * HOATOOLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with HOATOOLS.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * Guillermo A. Perez
 * University of Antwerp
 * guillermoalberto.perez@uantwerpen.be
 *************************************************************************/

/* C declarations */

/* Raise the bison parser stack cap from the 10k default so that HOA
 * files with thousands of states / transitions (e.g. the bulk outputs
 * from generators/generate_large.py) can be parsed without hitting a
 * premature "memory exhausted" error. 10M stack items is ~160MB worst
 * case, which is plenty on any machine that can hold the input file. */
#define YYMAXDEPTH 10000000

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "simplehoa.h"
#include "hoalexer.h"

// helper functions for the parser
void yyerror(const char* str) {
    fprintf(stderr, "Parsing error: %s [line %d]\n", str, yylineno);
}
 
int yywrap() {
    return 1;
}

bool autoError = false;
bool seenHeader[12] = {false};
const char* headerStrs[] = {"", "HOA", "Acceptance", "States", "AP",
                            "controllable-AP", "acc-name", "tool",
                            "name", "Start", "Alias", "properties"};

void hdrItemError(const char* str) {
    fprintf(stderr,
            "Automaton error: more than one %s header item [line %d]\n",
            str, yylineno - 1);  // FIXME: This is shifted for some reason
    autoError = true;
}

// where we will save everything parsed
static HoaData* loadedData;

// temporary internal structures for easy insertion, essentially
// singly linked lists
typedef struct StringList {
    char* str;
    struct StringList* next;
} StringList;

typedef struct IntList {
    int i;
    struct IntList* next;
} IntList;

typedef struct TransList {
    BTree* label;
    IntList* successors;
    IntList* accSig;
    struct TransList* next;
} TransList;

typedef struct StateList {
    int id;
    char* name;
    BTree* label;
    IntList* accSig;
    TransList* transitions;
    struct StateList* next;
    struct StateList* last;
} StateList;

typedef struct AliasList {
    char* alias;
    BTree* labelExpr;
    struct AliasList* next;
} AliasList;

static StringList* tempAps = NULL;
static StringList* tempAccNameParameters = NULL;
static StringList* tempProperties = NULL;
static IntList* tempStart = NULL;
static IntList* tempCntAPs = NULL;
static StateList* tempStates = NULL;
static AliasList* tempAliases = NULL;

// list management functions
static IntList* newIntNode(int val) {
    IntList* list = (IntList*) malloc(sizeof(IntList));
    list->i = val;
    list->next = NULL;
    return list;
}

static IntList* prependIntNode(IntList* node, int val) {
    IntList* newHead = (IntList*) malloc(sizeof(IntList));
    newHead->i = val;
    newHead->next = node;
    return newHead;
}

static StringList* prependStrNode(StringList* node, char* str) {
    StringList* newHead = (StringList*) malloc(sizeof(StringList));
    newHead->str = str;
    newHead->next = node;
    return newHead;
}

static StringList* concatStrLists(StringList* list1, StringList* list2) {
    if (list2 == NULL)
        return list1;
    if (list1 == NULL)
        return list2;

    StringList* cur = list1;
    while (cur->next != NULL)
        cur = cur->next;
    cur->next = list2;
    return list1;
}

static IntList* concatIntLists(IntList* list1, IntList* list2) {
    if (list2 == NULL)
        return list1;
    if (list1 == NULL)
        return list2;

    IntList* cur = list1;
    while (cur->next != NULL)
        cur = cur->next;
    cur->next = list2;
    return list1;
}

static char** simplifyStrList(StringList* list, int* cnt) {
    (*cnt) = 0;
    if (list == NULL) return NULL;
    StringList* cur = list;
    while (cur != NULL) {
        (*cnt)++;
        cur = cur->next;
    }
    // we copy them to a plain array while deleting nodes
    cur = list;
    int i = 0;
    char** dest = (char**) malloc(sizeof(char*) * (*cnt));
    while (cur != NULL) {
        StringList* next = cur->next;
        assert(cur->str != NULL);
        dest[i++] = cur->str;
        free(cur);
        cur = next;
    }
    return dest;
}

static int* simplifyIntList(IntList* list, int* cnt) {
    (*cnt) = 0;
    if (list == NULL) return NULL;
    IntList* cur = list;
    while (cur != NULL) {
        (*cnt)++;
        cur = cur->next;
    }
    // we copy them to a plain array while deleting nodes
    cur = list;
    int i = 0;
    int* dest = (int*) malloc(sizeof(int) * (*cnt));
    while (cur != NULL) {
        IntList* next = cur->next;
        dest[i++] = cur->i;
        free(cur);
        cur = next;
    }
    return dest;
}

// more list management functions
static StateList* newStateNode(int id, char* name, BTree* label, IntList* accSig) {
    StateList* list = (StateList*) malloc(sizeof(StateList));
    list->id = id;
    list->name = name;
    list->label = label;
    list->accSig = accSig;
    list->next = NULL;
    list->last = list;
    list->transitions = NULL;
    return list;
}

static StateList* appendStateNode(StateList* node, StateList* newNode,
                            TransList* transitions) {
    newNode->transitions = transitions;
    if (node == NULL) {
        return newNode; // nothing to do
    } else {
        // find last in list, add it
        node->last->next = newNode;
        node->last = newNode;
        return node;
    }
}

static TransList* prependTransNode(TransList* node , BTree* label,
                            IntList* successors, IntList* accSig) {
    TransList* newHead = (TransList*) malloc(sizeof(TransList));
    newHead->label = label;
    newHead->successors = successors;
    newHead->accSig = accSig;
    newHead->next = node;
    return newHead;
}

static AliasList* prependAliasNode(AliasList* node, char* alias, BTree* labelExpr) {
    AliasList* newHead = (AliasList*) malloc(sizeof(AliasList));
    newHead->alias = alias;
    newHead->next = node;
    newHead->labelExpr = labelExpr;
    newHead->next = node;
    return newHead;
}

static Transition* simplifyTransList(TransList* list, int* cnt) {
    (*cnt) = 0;
    if (list == NULL) return NULL;
    TransList* cur = list;
    while (cur != NULL) {
       (*cnt)++;
       cur = cur->next;
    }
    // we copy them to a plain array while deleting nodes
    cur = list;
    int i = 0;
    Transition* dest = (Transition*) malloc(sizeof(Transition) * (*cnt));
    while (cur != NULL) {
        TransList* next = cur->next;
        dest[i].label = cur->label;
        dest[i].successors = simplifyIntList(cur->successors,
                                             &(dest[i].noSucc));
        dest[i].accSig = simplifyIntList(cur->accSig,
                                         &(dest[i].noAccSig));
        i++;
        free(cur);
        cur = next;
    }
    return dest;
}

static State* simplifyStateList(StateList* list, int* cnt) {
    (*cnt) = 0;
    if (list == NULL) return NULL;
    StateList* cur = list;
    while (cur != NULL) {
        (*cnt)++;
        cur = cur->next;
    }
    // we copy them to a plain array while deleting nodes
    cur = list;
    int i = 0;
    State* dest = (State*) malloc(sizeof(State) * (*cnt));
    while (cur != NULL) {
        StateList* next = cur->next;
        dest[i].id = cur->id;
        dest[i].name = cur->name;
        dest[i].label = cur->label;
        dest[i].accSig = simplifyIntList(cur->accSig, &(dest[i].noAccSig));
        dest[i].transitions = simplifyTransList(cur->transitions,
                                                &(dest[i].noTrans));
        i++;
        free(cur);
        cur = next;
    }
    return dest;
}

static Alias* simplifyAliasList(AliasList* list, int* cnt) {
    (*cnt) = 0;
    if (list == NULL) return NULL;
    AliasList* cur = list;
    while (cur != NULL) {
        (*cnt)++;
        cur = cur->next;
    }
    // we copy them to a plain array while deleting nodes
    cur = list;
    int i = 0;
    Alias* dest = (Alias*) malloc(sizeof(Alias) * (*cnt));
    while (cur != NULL) {
        AliasList* next = cur->next;
        dest[i].alias = cur->alias;
        dest[i].labelExpr = cur->labelExpr;
        i++;
        free(cur);
        cur = next;
    }
    return dest;
}

// tree management functions
static BTree* boolBTree(bool b) {
    BTree* created = (BTree*) malloc(sizeof(BTree));
    created->left = NULL;
    created->right = NULL;
    created->alias = NULL;
    created->type = NT_BOOL;
    created->id = b ? 1 : 0;
    return created;
}

static BTree* andBTree(BTree* u, BTree* v) {
    BTree* created = (BTree*) malloc(sizeof(BTree));
    created->left = u;
    created->right = v;
    created->alias = NULL;
    created->type = NT_AND;
    created->id = -1;
    return created;
}

static BTree* orBTree(BTree* u, BTree* v) {
    BTree* created = (BTree*) malloc(sizeof(BTree));
    created->left = u;
    created->right = v;
    created->alias = NULL;
    created->type = NT_OR;
    created->id = -1;
    return created;
}

static BTree* notBTree(BTree* u) {
    BTree* created = (BTree*) malloc(sizeof(BTree));
    created->left = u;
    created->right = NULL;
    created->alias = NULL;
    created->type = NT_NOT;
    created->id = -1;
    return created;
}

static BTree* aliasBTree(char* alias) {
    BTree* created = (BTree*) malloc(sizeof(BTree));
    created->left = NULL;
    created->right = NULL;
    created->alias = alias;
    created->type = NT_ALIAS;
    created->id = -1;
    return created;
}

static BTree* apBTree(int id) {
    BTree* created = (BTree*) malloc(sizeof(BTree));
    created->left = NULL;
    created->right = NULL;
    created->alias = NULL;
    created->type = NT_AP;
    created->id = id;
    return created;
}

static BTree* accidBTree(NodeType type, int id, bool negated) {
    BTree* tree = (BTree*) malloc(sizeof(BTree));
    tree->left = NULL;
    tree->right = NULL;
    tree->alias = NULL;
    tree->type = NT_SET;
    tree->id = id;

    if (negated) {
        BTree* original = tree;
        tree = (BTree*) malloc(sizeof(BTree));
        tree->left = original;
        tree->right = NULL;
        tree->alias = NULL;
        tree->type = NT_NOT;
        tree->id = -1;
    }

    BTree* created = (BTree*) malloc(sizeof(BTree));
    created->left = tree;
    created->right = NULL;
    created->alias = NULL;
    created->type = type;
    created->id = -1;
    return created;
}


#line 482 "hoaparser.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "hoaparser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_HOAHDR = 3,                     /* HOAHDR  */
  YYSYMBOL_ACCEPTANCE = 4,                 /* ACCEPTANCE  */
  YYSYMBOL_STATES = 5,                     /* STATES  */
  YYSYMBOL_AP = 6,                         /* AP  */
  YYSYMBOL_CNTAP = 7,                      /* CNTAP  */
  YYSYMBOL_ACCNAME = 8,                    /* ACCNAME  */
  YYSYMBOL_TOOL = 9,                       /* TOOL  */
  YYSYMBOL_NAME = 10,                      /* NAME  */
  YYSYMBOL_START = 11,                     /* START  */
  YYSYMBOL_ALIAS = 12,                     /* ALIAS  */
  YYSYMBOL_PROPERTIES = 13,                /* PROPERTIES  */
  YYSYMBOL_LPAR = 14,                      /* "("  */
  YYSYMBOL_RPAR = 15,                      /* ")"  */
  YYSYMBOL_LBRACE = 16,                    /* "{"  */
  YYSYMBOL_RBRACE = 17,                    /* "}"  */
  YYSYMBOL_LSQBRACE = 18,                  /* "["  */
  YYSYMBOL_RSQBRACE = 19,                  /* "]"  */
  YYSYMBOL_BOOLOR = 20,                    /* "|"  */
  YYSYMBOL_BOOLAND = 21,                   /* "&"  */
  YYSYMBOL_BOOLNOT = 22,                   /* "!"  */
  YYSYMBOL_STATEHDR = 23,                  /* STATEHDR  */
  YYSYMBOL_INF = 24,                       /* INF  */
  YYSYMBOL_FIN = 25,                       /* FIN  */
  YYSYMBOL_BEGINBODY = 26,                 /* BEGINBODY  */
  YYSYMBOL_ENDBODY = 27,                   /* ENDBODY  */
  YYSYMBOL_STRING = 28,                    /* STRING  */
  YYSYMBOL_IDENTIFIER = 29,                /* IDENTIFIER  */
  YYSYMBOL_ANAME = 30,                     /* ANAME  */
  YYSYMBOL_HEADERNAME = 31,                /* HEADERNAME  */
  YYSYMBOL_INT = 32,                       /* INT  */
  YYSYMBOL_BOOL = 33,                      /* BOOL  */
  YYSYMBOL_YYACCEPT = 34,                  /* $accept  */
  YYSYMBOL_automaton = 35,                 /* automaton  */
  YYSYMBOL_header = 36,                    /* header  */
  YYSYMBOL_format_version = 37,            /* format_version  */
  YYSYMBOL_header_list = 38,               /* header_list  */
  YYSYMBOL_header_item = 39,               /* header_item  */
  YYSYMBOL_state_conj = 40,                /* state_conj  */
  YYSYMBOL_label_expr = 41,                /* label_expr  */
  YYSYMBOL_lab_exp_conj = 42,              /* lab_exp_conj  */
  YYSYMBOL_lab_exp_atom = 43,              /* lab_exp_atom  */
  YYSYMBOL_acceptance_cond = 44,           /* acceptance_cond  */
  YYSYMBOL_acc_cond_conj = 45,             /* acc_cond_conj  */
  YYSYMBOL_acc_cond_atom = 46,             /* acc_cond_atom  */
  YYSYMBOL_accid = 47,                     /* accid  */
  YYSYMBOL_boolintid_list = 48,            /* boolintid_list  */
  YYSYMBOL_boolintstrid_list = 49,         /* boolintstrid_list  */
  YYSYMBOL_string_list = 50,               /* string_list  */
  YYSYMBOL_id_list = 51,                   /* id_list  */
  YYSYMBOL_body = 52,                      /* body  */
  YYSYMBOL_statespec_list = 53,            /* statespec_list  */
  YYSYMBOL_state_name = 54,                /* state_name  */
  YYSYMBOL_maybe_label = 55,               /* maybe_label  */
  YYSYMBOL_maybe_string = 56,              /* maybe_string  */
  YYSYMBOL_maybe_accsig = 57,              /* maybe_accsig  */
  YYSYMBOL_int_list = 58,                  /* int_list  */
  YYSYMBOL_trans_list = 59                 /* trans_list  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  6
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   94

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  34
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  26
/* YYNRULES -- Number of rules.  */
#define YYNRULES  65
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  110

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   277


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   465,   465,   473,   475,   484,   485,   495,   499,   507,
     512,   516,   524,   529,   538,   539,   543,   551,   554,   555,
     558,   559,   562,   563,   566,   567,   568,   569,   570,   573,
     574,   577,   578,   581,   582,   583,   584,   587,   588,   591,
     592,   598,   604,   607,   608,   609,   610,   611,   613,   614,
     617,   618,   621,   625,   626,   632,   636,   637,   640,   641,
     644,   645,   648,   649,   652,   653
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "HOAHDR", "ACCEPTANCE",
  "STATES", "AP", "CNTAP", "ACCNAME", "TOOL", "NAME", "START", "ALIAS",
  "PROPERTIES", "\"(\"", "\")\"", "\"{\"", "\"}\"", "\"[\"", "\"]\"",
  "\"|\"", "\"&\"", "\"!\"", "STATEHDR", "INF", "FIN", "BEGINBODY",
  "ENDBODY", "STRING", "IDENTIFIER", "ANAME", "HEADERNAME", "INT", "BOOL",
  "$accept", "automaton", "header", "format_version", "header_list",
  "header_item", "state_conj", "label_expr", "lab_exp_conj",
  "lab_exp_atom", "acceptance_cond", "acc_cond_conj", "acc_cond_atom",
  "accid", "boolintid_list", "boolintstrid_list", "string_list", "id_list",
  "body", "statespec_list", "state_name", "maybe_label", "maybe_string",
  "maybe_accsig", "int_list", "trans_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-53)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-57)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      12,   -10,    20,    15,   -53,   -53,   -53,   -53,    -3,    23,
       7,    -6,    24,    25,    26,    30,    32,    33,    31,    35,
      37,   -53,   -53,   -53,    36,     4,     0,   -53,    34,    26,
     -53,    16,    39,   -53,    47,   -53,    -1,    37,   -53,    11,
      -1,    38,    31,   -53,     0,   -53,   -53,   -53,    49,    50,
     -53,    41,    34,   -53,   -53,    16,    16,    16,   -53,   -53,
     -53,    31,    -1,    -1,   -53,   -53,   -53,    52,    53,   -53,
     -53,   -53,   -53,   -53,   -53,    27,    39,    57,     3,     0,
       0,    -5,   -53,   -53,   -53,   -53,   -53,    22,   -53,    -1,
      -1,   -53,    57,    26,     4,   -53,    50,   -53,    43,    61,
     -53,    53,   -53,   -53,    60,   -53,    63,   -53,   -53,   -53
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     5,     4,     1,    53,     3,     0,
      52,     0,     0,     0,    62,     0,     0,     0,     0,     0,
      50,    43,     6,     2,    56,    64,     0,     7,    48,    62,
      10,    39,    58,    15,    18,     8,     0,    50,    16,    17,
       0,     0,     0,    54,     0,    38,    37,    36,    12,    29,
      31,     0,    48,     9,    63,    39,    39,    39,    13,    59,
      14,     0,     0,     0,    26,    25,    24,    11,    20,    22,
      51,    46,    47,    45,    44,     0,    58,    60,     0,     0,
       0,     0,    49,    42,    41,    40,    19,     0,    27,     0,
       0,    57,    60,    62,    64,    35,    30,    32,     0,     0,
      28,    21,    23,    55,     0,    65,     0,    33,    61,    34
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -53,   -53,   -53,   -53,   -53,   -53,   -26,   -28,    -9,   -52,
      40,     2,     5,   -53,    -4,   -53,    42,    45,   -53,   -53,
     -53,    55,    10,    -2,   -29,   -11
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     2,     3,     4,     8,    22,    35,    67,    68,    69,
      48,    49,    50,    51,    58,    39,    53,    38,     9,    10,
      25,    42,    60,    94,    30,    43
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      54,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    88,    75,    62,    44,     1,    77,    98,    95,     5,
       6,    63,    40,    79,    45,    46,    26,    99,    21,    64,
      24,    65,    66,    47,    87,    86,   -56,   100,   102,    71,
      72,     7,    89,    73,    74,    55,    91,    89,    56,    57,
      23,    83,    84,    85,    40,    81,    27,    28,    29,    31,
      32,    33,    52,    34,   104,    36,    37,    59,    61,    79,
      76,    80,    89,    93,    90,   106,   107,   108,   109,    41,
     101,    96,    70,   105,    78,    97,    92,     0,     0,     0,
     103,     0,     0,     0,    82
};

static const yytype_int8 yycheck[] =
{
      29,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    63,    40,    14,    14,     3,    42,    22,    15,    29,
       0,    22,    18,    20,    24,    25,    32,    32,    31,    30,
      23,    32,    33,    33,    62,    61,    32,    15,    90,    28,
      29,    26,    20,    32,    33,    29,    19,    20,    32,    33,
      27,    55,    56,    57,    18,    14,    32,    32,    32,    29,
      28,    28,    28,    32,    93,    30,    29,    28,    21,    20,
      32,    21,    20,    16,    21,    32,    15,    17,    15,    24,
      89,    79,    37,    94,    44,    80,    76,    -1,    -1,    -1,
      92,    -1,    -1,    -1,    52
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,    35,    36,    37,    29,     0,    26,    38,    52,
      53,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    31,    39,    27,    23,    54,    32,    32,    32,    32,
      58,    29,    28,    28,    32,    40,    30,    29,    51,    49,
      18,    55,    55,    59,    14,    24,    25,    33,    44,    45,
      46,    47,    28,    50,    58,    29,    32,    33,    48,    28,
      56,    21,    14,    22,    30,    32,    33,    41,    42,    43,
      51,    28,    29,    32,    33,    41,    32,    40,    44,    20,
      21,    14,    50,    48,    48,    48,    40,    41,    43,    20,
      21,    19,    56,    16,    57,    15,    45,    46,    22,    32,
      15,    42,    43,    57,    58,    59,    32,    15,    17,    15
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    34,    35,    36,    37,    38,    38,    39,    39,    39,
      39,    39,    39,    39,    39,    39,    39,    39,    40,    40,
      41,    41,    42,    42,    43,    43,    43,    43,    43,    44,
      44,    45,    45,    46,    46,    46,    46,    47,    47,    48,
      48,    48,    48,    49,    49,    49,    49,    49,    50,    50,
      51,    51,    52,    53,    53,    54,    55,    55,    56,    56,
      57,    57,    58,    58,    59,    59
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     4,     2,     2,     0,     2,     2,     2,     3,
       2,     3,     3,     3,     3,     2,     2,     2,     1,     3,
       1,     3,     1,     3,     1,     1,     1,     2,     3,     1,
       3,     1,     3,     4,     5,     3,     1,     1,     1,     0,
       2,     2,     2,     0,     2,     2,     2,     2,     0,     2,
       0,     2,     1,     0,     3,     5,     0,     3,     0,     1,
       0,     3,     0,     2,     0,     4
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 100000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* automaton: header BEGINBODY body ENDBODY  */
#line 466 "hoa.y"
         {
            if (!seenHeader[HOAHDR]) /* redundant because of the grammar */
                yyerror("No HOA: header item");
            if (!seenHeader[ACCEPTANCE])
                yyerror("No Acceptance: header item");
         }
#line 1716 "hoaparser.c"
    break;

  case 4: /* format_version: HOAHDR IDENTIFIER  */
#line 476 "hoa.y"
              {
                  loadedData->version = (yyvsp[0].string);
                  if (seenHeader[HOAHDR])
                      hdrItemError("HOA:");
                  else
                      seenHeader[HOAHDR] = true;
              }
#line 1728 "hoaparser.c"
    break;

  case 5: /* header_list: %empty  */
#line 484 "hoa.y"
                         { /* no new item, nothing to check */ }
#line 1734 "hoaparser.c"
    break;

  case 6: /* header_list: header_list header_item  */
#line 486 "hoa.y"
           {
               if ((yyvsp[0].number) <= 8) {
                   if (seenHeader[(yyvsp[0].number)])
                       hdrItemError(headerStrs[(yyvsp[0].number)]);
                   else
                       seenHeader[(yyvsp[0].number)] = true;
               }
           }
#line 1747 "hoaparser.c"
    break;

  case 7: /* header_item: STATES INT  */
#line 495 "hoa.y"
                                               {
                                                 loadedData->noStates = (yyvsp[0].number);
                                                 (yyval.number) = STATES;
                                               }
#line 1756 "hoaparser.c"
    break;

  case 8: /* header_item: START state_conj  */
#line 499 "hoa.y"
                                               {
                                                 tempStart =
                                                    concatIntLists(
                                                        tempStart,
                                                        (yyvsp[0].numlist)
                                                    );
                                                 (yyval.number) = START;
                                               }
#line 1769 "hoaparser.c"
    break;

  case 9: /* header_item: AP INT string_list  */
#line 507 "hoa.y"
                                               {
                                                 loadedData->noAPs = (yyvsp[-1].number);
                                                 tempAps = (yyvsp[0].strlist);
                                                 (yyval.number) = AP;
                                               }
#line 1779 "hoaparser.c"
    break;

  case 10: /* header_item: CNTAP int_list  */
#line 512 "hoa.y"
                                               {
                                                 tempCntAPs = (yyvsp[0].numlist);
                                                 (yyval.number) = CNTAP;
                                               }
#line 1788 "hoaparser.c"
    break;

  case 11: /* header_item: ALIAS ANAME label_expr  */
#line 516 "hoa.y"
                                               {
                                                 tempAliases =
                                                    prependAliasNode(
                                                        tempAliases,
                                                        (yyvsp[-1].string), (yyvsp[0].tree)
                                                    );
                                                 (yyval.number) = ALIAS;
                                               }
#line 1801 "hoaparser.c"
    break;

  case 12: /* header_item: ACCEPTANCE INT acceptance_cond  */
#line 524 "hoa.y"
                                               { 
                                                 loadedData->noAccSets = (yyvsp[-1].number);
                                                 loadedData->acc = (yyvsp[0].tree);
                                                 (yyval.number) = ACCEPTANCE;
                                               }
#line 1811 "hoaparser.c"
    break;

  case 13: /* header_item: ACCNAME IDENTIFIER boolintid_list  */
#line 529 "hoa.y"
                                               { 
                                                 loadedData->accNameID = (yyvsp[-1].string);
                                                 tempAccNameParameters
                                                    = concatStrLists(
                                                        tempAccNameParameters,
                                                        (yyvsp[0].strlist)
                                                    );
                                                 (yyval.number) = ACCNAME;
                                               }
#line 1825 "hoaparser.c"
    break;

  case 14: /* header_item: TOOL STRING maybe_string  */
#line 538 "hoa.y"
                                               { (yyval.number) = TOOL; }
#line 1831 "hoaparser.c"
    break;

  case 15: /* header_item: NAME STRING  */
#line 539 "hoa.y"
                                               {
                                                 loadedData->name = (yyvsp[0].string);
                                                 (yyval.number) = NAME;
                                               }
#line 1840 "hoaparser.c"
    break;

  case 16: /* header_item: PROPERTIES id_list  */
#line 543 "hoa.y"
                                               { 
                                                 tempProperties =
                                                     concatStrLists(
                                                         tempProperties,
                                                         (yyvsp[0].strlist)
                                                     );
                                                 (yyval.number) = PROPERTIES;
                                               }
#line 1853 "hoaparser.c"
    break;

  case 17: /* header_item: HEADERNAME boolintstrid_list  */
#line 551 "hoa.y"
                                               { free((yyvsp[-1].string)); (yyval.number) = HEADERNAME; }
#line 1859 "hoaparser.c"
    break;

  case 18: /* state_conj: INT  */
#line 554 "hoa.y"
                                { (yyval.numlist) = newIntNode((yyvsp[0].number)); }
#line 1865 "hoaparser.c"
    break;

  case 19: /* state_conj: INT "&" state_conj  */
#line 555 "hoa.y"
                                { (yyval.numlist) = prependIntNode((yyvsp[0].numlist), (yyvsp[-2].number)); }
#line 1871 "hoaparser.c"
    break;

  case 20: /* label_expr: lab_exp_conj  */
#line 558 "hoa.y"
                                        { (yyval.tree) = (yyvsp[0].tree); }
#line 1877 "hoaparser.c"
    break;

  case 21: /* label_expr: label_expr "|" lab_exp_conj  */
#line 559 "hoa.y"
                                        { (yyval.tree) = orBTree((yyvsp[-2].tree), (yyvsp[0].tree)); }
#line 1883 "hoaparser.c"
    break;

  case 22: /* lab_exp_conj: lab_exp_atom  */
#line 562 "hoa.y"
                                            { (yyval.tree) = (yyvsp[0].tree); }
#line 1889 "hoaparser.c"
    break;

  case 23: /* lab_exp_conj: lab_exp_conj "&" lab_exp_atom  */
#line 563 "hoa.y"
                                            { (yyval.tree) = andBTree((yyvsp[-2].tree), (yyvsp[0].tree)); }
#line 1895 "hoaparser.c"
    break;

  case 24: /* lab_exp_atom: BOOL  */
#line 566 "hoa.y"
                                 { (yyval.tree) = boolBTree((yyvsp[0].boolean)); }
#line 1901 "hoaparser.c"
    break;

  case 25: /* lab_exp_atom: INT  */
#line 567 "hoa.y"
                                 { (yyval.tree) = apBTree((yyvsp[0].number)); }
#line 1907 "hoaparser.c"
    break;

  case 26: /* lab_exp_atom: ANAME  */
#line 568 "hoa.y"
                                 { (yyval.tree) = aliasBTree((yyvsp[0].string)); }
#line 1913 "hoaparser.c"
    break;

  case 27: /* lab_exp_atom: "!" lab_exp_atom  */
#line 569 "hoa.y"
                                 { (yyval.tree) = notBTree((yyvsp[0].tree)); }
#line 1919 "hoaparser.c"
    break;

  case 28: /* lab_exp_atom: "(" label_expr ")"  */
#line 570 "hoa.y"
                                 { (yyval.tree) = (yyvsp[-1].tree); }
#line 1925 "hoaparser.c"
    break;

  case 29: /* acceptance_cond: acc_cond_conj  */
#line 573 "hoa.y"
                                                   { (yyval.tree) = (yyvsp[0].tree); }
#line 1931 "hoaparser.c"
    break;

  case 30: /* acceptance_cond: acceptance_cond "|" acc_cond_conj  */
#line 574 "hoa.y"
                                                   { (yyval.tree) = orBTree((yyvsp[-2].tree), (yyvsp[0].tree)); }
#line 1937 "hoaparser.c"
    break;

  case 31: /* acc_cond_conj: acc_cond_atom  */
#line 577 "hoa.y"
                                               { (yyval.tree) = (yyvsp[0].tree); }
#line 1943 "hoaparser.c"
    break;

  case 32: /* acc_cond_conj: acc_cond_conj "&" acc_cond_atom  */
#line 578 "hoa.y"
                                               { (yyval.tree) = andBTree((yyvsp[-2].tree), (yyvsp[0].tree)); }
#line 1949 "hoaparser.c"
    break;

  case 33: /* acc_cond_atom: accid "(" INT ")"  */
#line 581 "hoa.y"
                                       { (yyval.tree) = accidBTree((yyvsp[-3].nodetype), (yyvsp[-1].number), false); }
#line 1955 "hoaparser.c"
    break;

  case 34: /* acc_cond_atom: accid "(" "!" INT ")"  */
#line 582 "hoa.y"
                                       { (yyval.tree) = accidBTree((yyvsp[-4].nodetype), (yyvsp[-1].number), true); }
#line 1961 "hoaparser.c"
    break;

  case 35: /* acc_cond_atom: "(" acceptance_cond ")"  */
#line 583 "hoa.y"
                                       { (yyval.tree) = (yyvsp[-1].tree); }
#line 1967 "hoaparser.c"
    break;

  case 36: /* acc_cond_atom: BOOL  */
#line 584 "hoa.y"
                                       { (yyval.tree) = boolBTree((yyvsp[0].boolean)); }
#line 1973 "hoaparser.c"
    break;

  case 37: /* accid: FIN  */
#line 587 "hoa.y"
           { (yyval.nodetype) = NT_FIN; }
#line 1979 "hoaparser.c"
    break;

  case 38: /* accid: INF  */
#line 588 "hoa.y"
           { (yyval.nodetype) = NT_INF; }
#line 1985 "hoaparser.c"
    break;

  case 39: /* boolintid_list: %empty  */
#line 591 "hoa.y"
                                          { (yyval.strlist) = NULL; }
#line 1991 "hoaparser.c"
    break;

  case 40: /* boolintid_list: BOOL boolintid_list  */
#line 592 "hoa.y"
                                          { 
                                            (yyval.strlist) = (yyvsp[-1].boolean) ? prependStrNode((yyvsp[0].strlist),
                                                                     strdup("True"))
                                                : prependStrNode((yyvsp[0].strlist),
                                                                 strdup("False"));
                                          }
#line 2002 "hoaparser.c"
    break;

  case 41: /* boolintid_list: INT boolintid_list  */
#line 598 "hoa.y"
                                          {
                                            char buffer[66];
                                            sprintf(buffer, "%d", (yyvsp[-1].number));
                                            (yyval.strlist) = prependStrNode((yyvsp[0].strlist),
                                                                strdup(buffer));
                                          }
#line 2013 "hoaparser.c"
    break;

  case 42: /* boolintid_list: IDENTIFIER boolintid_list  */
#line 604 "hoa.y"
                                          { (yyval.strlist) = prependStrNode((yyvsp[0].strlist), (yyvsp[-1].string)); }
#line 2019 "hoaparser.c"
    break;

  case 47: /* boolintstrid_list: boolintstrid_list IDENTIFIER  */
#line 611 "hoa.y"
                                                { free((yyvsp[0].string)); }
#line 2025 "hoaparser.c"
    break;

  case 48: /* string_list: %empty  */
#line 613 "hoa.y"
                                { (yyval.strlist) = NULL; }
#line 2031 "hoaparser.c"
    break;

  case 49: /* string_list: STRING string_list  */
#line 614 "hoa.y"
                                { (yyval.strlist) = prependStrNode((yyvsp[0].strlist), (yyvsp[-1].string)); }
#line 2037 "hoaparser.c"
    break;

  case 50: /* id_list: %empty  */
#line 617 "hoa.y"
                            { (yyval.strlist) = NULL; }
#line 2043 "hoaparser.c"
    break;

  case 51: /* id_list: IDENTIFIER id_list  */
#line 618 "hoa.y"
                            { (yyval.strlist) = prependStrNode((yyvsp[0].strlist), (yyvsp[-1].string)); }
#line 2049 "hoaparser.c"
    break;

  case 52: /* body: statespec_list  */
#line 622 "hoa.y"
    { tempStates = (yyvsp[0].statelist); }
#line 2055 "hoaparser.c"
    break;

  case 53: /* statespec_list: %empty  */
#line 625 "hoa.y"
                            { (yyval.statelist) = NULL; }
#line 2061 "hoaparser.c"
    break;

  case 54: /* statespec_list: statespec_list state_name trans_list  */
#line 627 "hoa.y"
              {
                (yyval.statelist) = appendStateNode((yyvsp[-2].statelist), (yyvsp[-1].statelist), (yyvsp[0].trlist));
              }
#line 2069 "hoaparser.c"
    break;

  case 55: /* state_name: STATEHDR maybe_label INT maybe_string maybe_accsig  */
#line 633 "hoa.y"
          { (yyval.statelist) = newStateNode((yyvsp[-2].number), (yyvsp[-1].string), (yyvsp[-3].tree), (yyvsp[0].numlist)); }
#line 2075 "hoaparser.c"
    break;

  case 56: /* maybe_label: %empty  */
#line 636 "hoa.y"
                                { (yyval.tree) = NULL; }
#line 2081 "hoaparser.c"
    break;

  case 57: /* maybe_label: "[" label_expr "]"  */
#line 637 "hoa.y"
                                { (yyval.tree) = (yyvsp[-1].tree); }
#line 2087 "hoaparser.c"
    break;

  case 58: /* maybe_string: %empty  */
#line 640 "hoa.y"
                          { (yyval.string) = NULL; }
#line 2093 "hoaparser.c"
    break;

  case 59: /* maybe_string: STRING  */
#line 641 "hoa.y"
                          { (yyval.string) = (yyvsp[0].string); }
#line 2099 "hoaparser.c"
    break;

  case 60: /* maybe_accsig: %empty  */
#line 644 "hoa.y"
                               { (yyval.numlist) = NULL; }
#line 2105 "hoaparser.c"
    break;

  case 61: /* maybe_accsig: "{" int_list "}"  */
#line 645 "hoa.y"
                               { (yyval.numlist) = (yyvsp[-1].numlist); }
#line 2111 "hoaparser.c"
    break;

  case 62: /* int_list: %empty  */
#line 648 "hoa.y"
                       { (yyval.numlist) = NULL; }
#line 2117 "hoaparser.c"
    break;

  case 63: /* int_list: INT int_list  */
#line 649 "hoa.y"
                       { (yyval.numlist) = prependIntNode((yyvsp[0].numlist), (yyvsp[-1].number)); }
#line 2123 "hoaparser.c"
    break;

  case 64: /* trans_list: %empty  */
#line 652 "hoa.y"
                        { (yyval.trlist) = NULL; }
#line 2129 "hoaparser.c"
    break;

  case 65: /* trans_list: maybe_label state_conj maybe_accsig trans_list  */
#line 654 "hoa.y"
          { (yyval.trlist) = prependTransNode((yyvsp[0].trlist), (yyvsp[-3].tree), (yyvsp[-2].numlist), (yyvsp[-1].numlist)); }
#line 2135 "hoaparser.c"
    break;


#line 2139 "hoaparser.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 657 "hoa.y"

/* Additional C code */
  
int parseHoa(FILE* input, HoaData* data) {
    loadedData = data;
    yyin = input;
    int ret = yyparse();

    // Last (semantic) checks:
    bool semanticError = false;
    // let us check that the number of APs makes sense
    int noAPs = 0;
    for (StringList* it = tempAps; it != NULL; it = it->next)
        noAPs++;
    if (noAPs != loadedData->noAPs) {
        fprintf(stderr,
                "Semantic error: the number and list of atomic propositions "
                "do not match (%d vs %d)\n", noAPs, loadedData->noAPs);
        semanticError = true;
    }

    // clean up internal handy elements
    loadedData->aps = simplifyStrList(tempAps, &(loadedData->noAPs));
    loadedData->accNameParameters = simplifyStrList(tempAccNameParameters,
                                                    &(loadedData->noANPs));
    loadedData->properties = simplifyStrList(tempProperties,
                                             &(loadedData->noProps));
    loadedData->start = simplifyIntList(tempStart, &(loadedData->noStart));
    loadedData->cntAPs = simplifyIntList(tempCntAPs, &(loadedData->noCntAPs));
    loadedData->states = simplifyStateList(tempStates, &(loadedData->noStates));
    loadedData->aliases = simplifyAliasList(tempAliases,
                                            &(loadedData->noAliases));
    
    return ret | autoError | semanticError;
}
