#!/usr/bin/env perl

use strict;

my %stdio = (
    stdin  => [],
    stdout  => [],
    stderr => []
);

while (my $line = <>) {
    chomp $line;
    $line =~ s/^args /arguments /;
    if ($line =~ m/^file-new ([^ ]*) ([^ ]*)/) {
        $line = "file $1 {} $2";
    }
    elsif ($line =~ m/^file-del ([^ ]*) ([^ ]*)/) {
        $line = "file $1 $2 {}";
    }
    elsif ($line =~ m/^file ([^ ]*) ([^ ]*) \2/) {
        $line = "file $1 $2";
    }
    elsif ($line =~ m/^(stdin|stdout|stderr) (.*)/) {
        push @{$stdio{$1}}, $2;
        next;
    }
    elsif ($line =~ m/^(stdin|stdout|stderr)/) {
        push @{$stdio{$1}}, "";
        next;
    }
    elsif ($line =~ m/^(mkdir) [^ ]* (.*)/) {
        $line = "$1 $2";
    }
    print "$line\n";
}

print_stdio("stdin");
print_stdio("stdout");
print_stdio("stderr");

sub print_stdio {
    my ($name) = @_;

    if (scalar(@{$stdio{$name}}) > 0) {
        print "$name\n";
        print join "\n", @{$stdio{$name}};
        print "\nend-of-inline-data\n";
    }
}
