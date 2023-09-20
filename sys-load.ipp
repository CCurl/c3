#define __SYS_LOAD__

#ifndef __SYS_LOAD__

void sysLoad() {}

#else

void ML2(const char *name, char op1, char op2) {
    doCreate((char*)name);
    CComma(op1); CComma(op2); CComma(EXIT);
    last->f = (op2 == ENDWORD) ? IS_IMMEDIATE : IS_INLINE;
}

void sysLoad() {
    ML2("INLINE",    SYS_OPS, INLINE);
    ML2("IMMEDIATE", SYS_OPS, IMMEDIATE);
    ML2("(.)",       SYS_OPS, DOT);
    ML2("ITOA",      SYS_OPS, ITOA);
    ML2(":",         SYS_OPS, DEFINE);
    ML2(";",         SYS_OPS, ENDWORD);
    ML2("CREATE",    SYS_OPS, CREATE);
    ML2("'",         SYS_OPS, FIND);
    ML2("NEXT-WORD", SYS_OPS, WORD);
    ML2("TIMER",     SYS_OPS, TIMER);
    ML2(",",         SYS_OPS, COMMA);
    ML2("C,",        SYS_OPS, CCOMMA);
    ML2("KEY",       SYS_OPS, KEY);
    ML2("?KEY",      SYS_OPS, QKEY);
    ML2("EMIT",      SYS_OPS, EMIT);
    ML2("TYPEZ",     SYS_OPS, TYPEZ);
    ML2("S-TRUNC",   STR_OPS, TRUNC);
    ML2("S-CPY",     STR_OPS, STRCPY);
    ML2("S-CAT",     STR_OPS, STRCAT);
    ML2("S-LEN",     STR_OPS, STRLEN);
    ML2("S-EQ",      STR_OPS, STREQ);
    ML2("S-EQ-I",    STR_OPS, STREQI);
    ML2("F+", FLT_OPS, FADD);
    ML2("F-", FLT_OPS, FSUB);
    ML2("F*", FLT_OPS, FMUL);
    ML2("F/", FLT_OPS, FDIV);
    ML2("F.", FLT_OPS, FDOT);
}

#endif
