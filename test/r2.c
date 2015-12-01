
    char	c[] = {
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'
    };
main()
{

    int		q, n;
    int		fd;

    fd = open ( "foo", FCREAT | FWRITE, 0644 );

    n = 0;
    for ( q = 0; q < 5; q ++ ) {
	n = n + write ( fd, c, 10 );
    }
    printf ( "N = %d\n", n );
    close (fd);
    exit(4);
}
