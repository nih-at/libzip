#!/usr/bin/env perl

use strict;

my @files = @ARGV;

my $docset = 'at.nih.libzip.docset';
my @sh_nodes = qw(zipcmp zipmerge ziptorrent);

(system('rm', '-rf', $docset) == 0) or die "can't remove old version of docset: $!";

mkdir($docset) or die "can't create docset directory: $!";
mkdir("$docset/Contents") or die "can't create docset directory: $!";
mkdir("$docset/Contents/Resources") or die "can't create docset directory: $!";
mkdir("$docset/Contents/Resources/Documents") or die "can't create docset directory: $!";

open I, "> $docset/Contents/Info.plist" or die "can't create Info.plist: $!";
print I <<EOF;
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleIdentifier</key>
	<string>at.nih.libzip.docset</string>
	<key>CFBundleName</key>
	<string>libzip</string>
	<key>DocSetPublisherIdentifier</key>
	<string>at.nih</string>
	<key>DocSetPublisherName</key>
	<string>NiH</string>
        <key>NSHumanReadableCopyright</key>
        <string>Copyright © 2012 Dieter Baron and Thomas Klausner</string>
</dict>
</plist>
EOF
close I;

open N, "> $docset/Contents/Resources/Nodes.xml" or die "can't create Nodes.xml: $!";

my %tl_nodes = (libzip => 1);

for (@sh_nodes) {
    $tl_nodes{$_} = 1;
}

my @tokens = ({ type => '//apple_ref/c/func',
		path => 'libzip.html',
		description => 'C library for reading, creating, and modifying zip archives',
		id => 1 });

print N <<EOF;
<?xml version="1.0" encoding="UTF-8"?>
<DocSetNodes version="1.0">
  <TOC>
    <Node id="1" type="folder">
      <Name>libzip</Name>
      <Path>libzip.html</Path>
      <SubNodes>
EOF

    
my $id = 1001;

for my $html (@files) {
    my $name = $html;
    $name =~ s/.html//;
    my $mdoc = "$name.mdoc";

    if ($tl_nodes{$name}) {
	next;
    }

    my $description;
    my @names = ();

    open MD, "< $mdoc" or die "can't open $mdoc: $!";

    while (my $line = <MD>) {
	if ($line =~ m/^.Nm (.*?)( ,)?$/) {
	    push @names, $1;
	}
	elsif ($line =~ m/^.Nd (.*)/) {
	    $description = $1;
	}
	elsif ($line =~ m/^.Sh SYNOPSIS/) {
	    last;
	}
    }

    close MD;

    print N "        <Node id=\"$id\">\n";
    print N "          <Name>$name</Name>\n";
    print N "          <Path>$html</Path>\n";
    print N "        </Node>\n";

    for my $name (@names) {
	push @tokens, { type => '//apple_ref/c/func',
			path => $html,
			name => $name,
			description => $description,
			id => $id };
    }
    
    copy_html($html, "$docset/Contents/Resources/Documents/$html");

    $id++;
}

print N "      </SubNodes>\n";
print N "    </Node>\n";

print N <<EOF;
  </TOC>
</DocSetNodes>
EOF

close N;

link('nih-man.css', "$docset/Contents/Resources/Documents/nih-man.css") or die "can't link css file: $!";
copy_html('libzip.html', "$docset/Contents/Resources/Documents/libzip.html");

system('docsetutil', 'index', $docset) == 0 or die "can't index docset: $!";
system('docsetutil', 'validate', $docset) == 0 or die "can't verify docset: $!";



sub copy_html {
    my ($src, $dst) = @_;

    my $content = `cat $src`;
    $content =~ s,</head>,<meta name="viewport" content="width=device-width"></head>,;
    $content =~ s,../nih-man.css,nih-man.css,;

    $content =~ s,(<div class="sec-body") style="margin-left: 5.00ex;">,$1>,g;
    $content =~ s,<table class="(header|footer)".*?</table>,,sg;

    open X, "> $dst" or die "can't create $dst: $!";
    print X $content;
    close X;
}

sub write_token {
    my ($T, $token) = @_;

    print $T "  <Token>\n";
    print $T "    <TokenIdentifier>$token->{type}/$token->{name}</TokenIdentifier>\n";
    print $T "    <Path>$token->{path}</Path>\n";
    print $T "    <Abstract>$token->{description}</Abstract>\n";
    print $T "    <NodeRef refid=\"$token->{id}\" />\n";
    print $T "  </Token>\n";
}

sub write_tokens {
    open my $T, "> $docset/Contents/Resources/Tokens.xml" or die "can't create Tokens.xml: $!";
    print $T "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    print $T "<Tokens version=\"1.0\">\n";

    for my $token (sort { $a->{name} cmp $b->{name} } @tokens) {
	write_token($T, $token);
    }

    print $T "</Tokens>\n";

    close $T;
}
