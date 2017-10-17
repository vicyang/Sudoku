use Encode;
use IO::Handle;
STDOUT->autoflush(1);
use File::Slurp;

print "loading..";
my @arr = read_file("a.txt");
my @brr = read_file("b.txt");

print "Done\n";

print "Sorting...";
@arr = sort { $a cmp $b } @arr;
@brr = sort { $a cmp $b } @brr;
print "Done\n";

write_file( "aa.txt", @arr );
write_file( "bb.txt", @brr );


