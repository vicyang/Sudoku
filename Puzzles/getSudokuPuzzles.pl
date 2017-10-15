=info
    523066680 2017-09
    https://zhuanlan.zhihu.com/PerlExample
=cut

use threads;
use threads::shared;
use IO::Handle;
use File::Slurp;
use Data::Dumper;
use Time::HiRes qw/sleep time/;
STDOUT->autoflush(1);
 
$Data::Dumper::Indent = 1;
$Data::Dumper::Sortkeys = 1;

use Date::Format;
use Time::Local;
use LWP::UserAgent;

our $level = 0;
our $dbfile;
our $main = "http://www.cn.sudokupuzzle.org";
our $ua = LWP::UserAgent->new(
            agent => "Mozilla/5.0", keep_alive => 1, timeout => 5,
         );

our $db    :shared;
our @tasks :shared;
my @ths;

while ( $level <= 4 )
{
    $dbfile = "sudoku_nd${level}.txt";
    $db = undef;
    @tasks = ();

    #结构初始化，设置任务列表
    initdb( \$db, \@tasks, $dbfile );

    @ths = ();
    grep { push @ths, threads->create( \&getSudoku, $_ ); } (0..4);

    #等待并结束线程
    while ( threads->list( threads::running ) ) { sleep 1.0 }
    grep { $_->detach() } @ths;

    #最终数据输出
    write_file( $dbfile, Dumper $db );
    print "Done $dbfile\n";

    $level++;
}

sub getSudoku
{
    our ($main, $ua, $level, $db, @tasks);
    my $id = shift;
    my $html;
    my $link;
    my $res;
    my $timestamp;
    my ($year, $mon, $day);

    while ( $#tasks >= 0 )
    {
        $timestamp = shift @tasks;
        ($year, $mon, $day) = split( " ", time2str( "%Y %L %e", $timestamp ) );
        $link = "$main/printable.php?nd=$level&y=$year&m=$mon&d=$day";
        GET: while (1)
        {
            $res  = $ua->get($link);
            $html = $res->content();

            if ($html=~/(\d{162})/)
            {
                $db->{ $timestamp } = $1;
                printf "[%d] %d-%02d-%02d Done\n", $id, $year, $mon, $day;
                last GET;
            }
        }

        #遇到偶数月份零一日的时候保存数据，避免程序中断前功尽弃
        if ( ($day == 1) and ($mon % 2 == 0) )
        {
            write_file( $dbfile, Dumper $db );
        }
    }
}

sub initdb
{
    our ($main, $level);
    my ( $db, $tasks, $dbfile ) = @_;

    my $iter = timelocal(0, 0, 0, 1, 0, 2008 );  #起点日期，月份从0开始
    my $last = time();                           #最后日期为今天
    my %new_db;

    my $old_db;
    $old_db = eval read_file( $dbfile ) if ( -e $dbfile );

    while ( $iter < $last )
    {
        if ( defined $old_db->{$iter} )
        {
            $new_db{$iter} = $old_db->{$iter};
        }
        else
        {
            $new_db{$iter} = undef;
            push @$tasks, $iter;
        }

        $iter += 24*3600;
    }

    $$db = shared_clone( \%new_db );
}