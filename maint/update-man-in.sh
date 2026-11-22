#! /bin/sh
#
# Generate doc/man/roff/*.1.in from doc/man/*.md.
#
# Needs go-md2man and perl.  It is a maintainer tool: the generated files
# are in the repository, so building from a checkout needs neither.
#
# Run it from the top of the source tree, or with the tree as an argument.

set -e

top=${1-.}
MD2MAN=${MD2MAN-go-md2man}
PERL=${PERL-perl}
SED=${SED-sed}

for tool in "$MD2MAN" "$PERL"; do
    if ! command -v "$tool" > /dev/null 2>&1; then
        echo "$0: $tool not found" >&2
        exit 1
    fi
done

# Every page is of section 1 and of this manual; only the date is its own,
# and it stands in the markdown, in front of the text it belongs to.
SECTION=1
SOURCE="bluerig {{MAN_VERSION}}"
MANUAL="bluerig"

for md in "$top"/doc/man/*.md "$top"/doc/man/*/*.md; do
    [ -f "$md" ] || continue

    name=`basename "$md" .md`
    out=`dirname "$md"`/roff/$name.$SECTION.in
    title=`echo "$name" | tr '[a-z]' '[A-Z]'`
    date=`$SED -n 's/^date: *//p' "$md" | head -n 1`

    if [ -z "$date" ]; then
        echo "$0: $md: no date in the front matter" >&2
        exit 1
    fi

    mkdir -p "`dirname \"$out\"`"
    echo ".\\\" Generated from $name.md by maint/update-man-in.sh. Do not edit." \
        > "$out.tmp"

    # go-md2man wants the title of the page in its own first line, and the
    # front matter would end up as a rule and a heading, so neither reaches it
    { echo "% $title $SECTION \"$date\" \"$SOURCE\" \"$MANUAL\""
      $SED -e '1,/^---$/d' "$md"
    } | $PERL -e '
        # a table of two columns, as the help window shows it, has no roff
        # without tbl: it becomes a list of terms, the first cell the term;
        # a head with no text is left out, and a row with no term is one
        # more line of the term above it
        my @l = <STDIN>;
        # {{key:Action|keys}}: the help window shows the keys bound to the
        # action now; a man page has the default keys after the bar
        s/\{\{key:[A-Za-z0-9_]+\|(.*?)\}\}/$1/g for @l;
        sub cells {
            my $row = shift;
            chomp $row;
            $row =~ s/^\|//;
            $row =~ s/(?<!\\)\|\s*$//;
            return map { my $c = $_; $c =~ s/\\\|/|/g; $c =~ s/^\s+|\s+$//g; $c }
                   split /(?<!\\)\|/, $row, -1;
        }
        for (my $i = 0; $i < @l; $i++) {
            if ($l[$i] =~ /^\|/ && $i + 1 < @l && $l[$i + 1] =~ /^\|[\s|:]*-[\s|:-]*$/) {
                my @rows = ($l[$i]);
                for ($i += 2; $i < @l && $l[$i] =~ /^\|/; $i++) {
                    push @rows, $l[$i];
                }
                $i--;
                my $open = 0;
                for my $k (0 .. $#rows) {
                    my @c = cells($rows[$k]);
                    next if $k == 0 && !grep { $_ ne "" } @c;
                    if ($c[0] ne "" || !$open) {
                        print "\n" if $open;
                        print "**$c[0]**\n";
                        $open = 1;
                    }
                    print ": $c[1]\n";
                }
                print "\n";
            } else {
                print $l[$i];
            }
        }
    ' | $MD2MAN | $PERL -0777 -pe '
        # go-md2man says what it can in roff, and these five rules say the
        # rest.  Order matters: an internal link must go before rule 4, or
        # it would be turned into a url.

        # 1. .nh turns hyphenation off for the whole page, which refills
        #    every paragraph of it
        s/\A\.nh\n//;

        # 2. the anchor of a help node is html, and what it leaves behind
        #    is an empty paragraph
        s/\n\.PP\n\n+(?=\.[SU][HS])/\n/g;

        # 3. a link to a help node has no url to print, only its text; the node may
        #    live in another help page
        s/\n?\\\[la\](?:[A-Za-z0-9_-]+\.md)?#(?:[^\\]|\\.)*?\\\[ra\]//g;

        # 4. a link to the outside is clickable where OSC 8 is understood
        s{\n+\\\[la\](\S+)\\\[ra\]}{\n\\X'"'"'tty: link $1'"'"'$1\\X'"'"'tty: link'"'"'}g;

        # 5. a hyphen is a minus sign: it prints the same and keeps groff
        #    from breaking a word at it
        s/(?<!\\)-/\\-/g;

        # 6. roff reads an apostrophe at the start of a line as a control
        #    character
        s/^'"'"'/\\&'"'"'/gm;

        # 7. what the anchor of a heading leaves behind it
        s/[ \t]+$//gm;

        # 8. a backslash of its own is \e in these pages; \\ is what mandoc
        #    tells man pages not to write
        s/\\\\/\\e/g;

        # 9. a code block that ends with an empty line: go-md2man 2.0.3 keeps
        #    the line before .EE, 2.0.4 does not; neither does this page
        s/\n\n+(?=\.EE\n)/\n/g;
    ' >> "$out.tmp"
    mv -f "$out.tmp" "$out"
    echo "$out"
done
