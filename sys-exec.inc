#ifdef isPC

        NCASE SYSTEM: t1=pop(); system(ToCP(t1+1));
        NCASE FOPEN:  t1=pop(); TOS=(cell_t)fopen(ToCP(TOS+1), ToCP(t1+1));
        NCASE FCLOSE: t1=pop(); fclose((FILE*)t1);
        NCASE FREAD:  t1=pop(); n1=pop(); TOS =  fread(ToCP(TOS), 1, n1, (FILE*)t1);
        NCASE FWRITE: t1=pop(); n1=pop(); TOS = fwrite(ToCP(TOS), 1, n1, (FILE*)t1);
        NCASE FLOAD:  y=ToCP(pop()); t1=(cell_t)fopen(y+1, "rt");
                if (t1 && input_fp) { fileStk[++fileSp]=input_fp; }
                if (t1) { input_fp = t1; ClearTib; }
                else { PRINT3("-noFile[",y+1,"]-"); }

#else

#endif
