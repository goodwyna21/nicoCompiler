(x++);
x++;
++x;
x+1;
(y+1)+x; //<- broken
y+(1+x); //<- works
y+
2;

///
skip this
a++;
///

//skip this comment
return 1; //skip this comment
return a[b[1][2]][3];
return a[; //should throw error
++((a+); //should throw error
return a[3+(1+2)];