#define    permrange 6
int    permarray[permrange+1];
int    pctr;

    /* Permutation program, heavily recursive, written by Denny Brown. */

    Swap ( a,b )
	int *a, *b;
	{
	int t;
	t = *a;  *a = *b;  *b = t;
	};

    Initialize ()
	{
	int i;
	for ( i = 1; i <= 7; i++ ) {
	    permarray[i]=i-1;
	    };
	};

    Permute (n)
	int n;
	{   /* permute */
	int k;
	pctr = pctr + 1;
	if ( n!=1 )  {
	    Permute(n-1);
	    for ( k = n-1; k >= 1; k-- ) {
		Swap(&permarray[n],&permarray[k]);
		Permute(n-1);
		Swap(&permarray[n],&permarray[k]);
		};
	    };
	}     /* permute */;

Perm ()    {   /* Perm */
    int i;
    pctr = 0;
	Initialize();
	Permute(7);
    }     /* Perm */;

main()
{
Perm();
}
