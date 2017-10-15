=info
    523066680@163.com
    2017-09
=cut

use IO::Handle;
STDOUT->autoflush(1);

my $main = 0;
my $bad = 0;

AGAIN:
our %hash;
grep { $hash{$_} = 1 } (1..9);
@nums = (1..9);

our $mat = 
    [
		#[ 1,2,3,4,5,6,7,8,9 ],
		[map { splice @nums, int(rand($#nums+1)), 1 } (1..9)],
        [],[],[],[],[],[],[],[]
    ];

my $nextline;
for my $line ( 1 .. 8 )
{
    $nextline = undef;

    func( $line, 0, \$nextline );
    if ( not defined $nextline )
    {
    	$bad++;
        print "not ok\n";
    }

    $mat->[$line] = [ split("", $nextline) ];
}

for my $idx ( 0..$#$mat )
{
    printf "%s\n", join(",", @{$mat->[$idx]} );
}

print "\n";
goto AGAIN if ($main++ < 1000);
printf "bad: %d\n", $bad;

sub func
{
    our @all;
    our $mat;
    our %hash;
    my ( $curr, $lv, $ref ) = @_;

    if ($lv > 8)
    {        
        #如果当前行是第五行或者第八行，预判下一行是否无解
        if ( $curr == 4 or $curr == 7)
        {
        	if ( try_next_line( $curr ) == 0 )
        	{
        		return 0;
        	}
        	else
        	{
        		$$ref = join("", @{$mat->[$curr]});
        		return 1;
        	}
    	}
    	else
    	{
    		$$ref = join("", @{$mat->[$curr]});
        	return 1;
    	}
    }

    my @out;
    my %dupl;

    if ( $curr % 3 == 1 )  # 1 2  4 5  7 8
    {
        if    ( $lv < 3 ) { @out = @{$mat->[$curr-1]}[0..2]; }
        elsif ( $lv < 6 ) { @out = @{$mat->[$curr-1]}[3..5]; }
        else              { @out = @{$mat->[$curr-1]}[6..8]; }
    }
    elsif ( $curr % 3 == 2 )
    {
        if    ( $lv < 3 ) { @out = (@{$mat->[$curr-1]}[0..2], @{$mat->[$curr-2]}[0..2]) }
        elsif ( $lv < 6 ) { @out = (@{$mat->[$curr-1]}[3..5], @{$mat->[$curr-2]}[3..5]) }
        else              { @out = (@{$mat->[$curr-1]}[6..8], @{$mat->[$curr-2]}[6..8]) }
    }

    if ($curr >= 3)
    {
        push @out, map { $mat->[$_][$lv] } ( 0 .. 3*int($curr/3) - 1 );
    }
    
    push @out, @{$mat->[$curr]}[ 0 .. $lv-1 ];

    %dupl = %hash;
    grep { delete $dupl{$_} } @out;

    my $res = 0;
    for my $e ( keys %dupl )
    {
        $mat->[$curr][$lv] = $e;
        $res = func($curr, $lv+1, $ref);

        last if ($res == 1);
    }

    return $res;
}

sub try_next_line
{
	my ($row) = shift;
    my $nextline = undef;
    func( $row+1, 0, \$nextline );
    if ( not defined $nextline )
    {
        return 0;
    }
    else
    {
    	return 1;
    }
}
