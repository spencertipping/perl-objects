#!/usr/bin/perl
# 99aeabc9ec7fe80b1b39f5e53dc7e49e      <- self-modifying Perl magic
# state:  Cz7culwqv6kV3bqpFudqQzwMpdO1oCFI6H7wTnpNm9k
# istate: wfbw15pcd3yHBY5soXnZgMMsIpVqV+I55h3vdgIvf/g
# id:     246bc56c88e8e8daae3737dbb16a2a2c

# This is a self-modifying Perl file. I'm sorry you're viewing the source (it's
# really gnarly). If you're curious what it's made of, I recommend reading
# http://github.com/spencertipping/writing-self-modifying-perl.
#
# If you got one of these from someone and don't know what to do with it, send
# it to spencer@spencertipping.com and I'll see if I can figure out what it
# does.

# For the benefit of HTML viewers (this is a hack):
# <div id='cover' style='position: absolute; left: 0; top: 0; width: 10000px; height: 10000px; background: white'></div>

$|++;

my %data;
my %transient;
my %externalized_functions;
my %datatypes;

my %locations;          # Maps eval-numbers to attribute names

my $global_data = join '', <DATA>;

sub meta::define_form {
  my ($namespace, $delegate) = @_;
  $datatypes{$namespace} = $delegate;
  *{"meta::${namespace}::implementation"} = $delegate;
  *{"meta::$namespace"} = sub {
    my ($name, $value, %options) = @_;
    chomp $value;
    $data{"${namespace}::$name"} = $value unless $options{no_binding};
    &$delegate($name, $value) unless $options{no_delegate}}}

sub meta::eval_in {
  my ($what, $where) = @_;

  # Obtain next eval-number and alias it to the designated location
  @locations{eval('__FILE__') =~ /\(eval (\d+)\)/} = ($where);

  my $result = eval $what;
  $@ =~ s/\(eval \d+\)/$where/ if $@;
  warn $@ if $@;
  $result}

meta::define_form 'meta', sub {
  my ($name, $value) = @_;
  meta::eval_in($value, "meta::$name")};
eval `perl object serialize -p`; die $@ if $@;
meta::meta('type::js', 'meta::functor::editable \'js\', extension => \'.js\', inherit => 1;');
meta::cache('parent-identification', 'object 99aeabc9ec7fe80b1b39f5e53dc7e49e');
meta::cache('parent-state', '99aeabc9ec7fe80b1b39f5e53dc7e49e lKiIRzqawBQkDkNijhtz/y20IXLrf041WaoMhS6zsKk');
meta::data('author', 'Spencer Tipping');
meta::data('default-action', 'shell');
meta::data('license', <<'__');
MIT License
Copyright (c) 2010 Spencer Tipping

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
__
meta::data('permanent-identity', '246bc56c88e8e8daae3737dbb16a2a2c');
meta::data('save-indirect', '1');
meta::parent('object', <<'__');
bootstrap::html                         /9nKOzNlCVjt5KpPCw6ktpNdorICJPOmdsxw8gRbQgs
bootstrap::initialization               TvbsjnluOC8KVyAVtB4a6Yzd+QlZ0Caz+PywmbS+RsE
bootstrap::perldoc                      Yun55m+nTQKU5l0UCuImJQGrDnrhsJ18adHGKgtzQ6M
function::ad                            TDzQEoFVHn4cxoMC1v2lQtvbymt7TDA+fzzf8ZdKBTY
function::alias                         76cEB7oBB2F8XdFTKnJy1lDh45nPgZ0c0xeoQUesLDk
function::cat                           6DwhwhDw7GTCE8Ll4Hwi13L3agMbCK7nbX0tDdC4MNU
function::cc                            YUoFsxNwVjmiHsdz9J7A13fBhp0++EiinFxETzF8+3g
function::ccc                           T3ySqmDsgIDdk55Fm4agtCEe1OOYWihhUcfiPtTupAs
function::cd                            cH/fs1az9tELnBgTC7JW/eb/ZSsf7s+K8Os8cMCeEG8
function::child                         y8XMjuUj//66Rj5jSs4HdEn1SqEIT1UVO6/2zb12Ln8
function::clone                         3ImmJEgQg9iM1gqe/hfKBFBm/Ky0HR/+xWj6XLXF3xk
function::cp                            +586T+V2LWthllDZvvTJzGNtngNo5c1pcS36XgTh1gs
function::create                        EG9exU+k3oTxSMX2TJHp660yAjhAe7+KZd4qBC1+vR8
function::current-state                 BPu6xpOyr3Ns0vYdawRlNxsmeRPITGp1bm0QtMaXFw4
function::cwd                           kUuYtqZpgzhXPJOe4f+SLRCFTG+tNl5j8p7+0yqE6C0
function::disable                       ETVjo3v6gmGncp7YgrEmQiDlF7GTHP0EX3yMOsn0M98
function::edit                          QgBU/tNoicpE+PHlVDKuUZ5vn5Da36GrycIxE0MwbA0
function::edit-self                     CnLk1bZDZ7VYrDQlYzfHXZ1aP5+0FrGCRBU1CUhel5I
function::enable                        uj6pBBIjv0ahKWcnbyL1IU4bsvDQqVJrQt6LrsUMzc4
function::expanded-bootstrap            20atbt3JeNY20woVBrxVH1IRnx/Sk8gPrY/nayXBIvs
function::export                        a/OWMZFsFsvHwnlvCBEZMg8WhZ9IP9ZwqJZBj7Garq8
function::extern                        EG6Lhkqo4xNy3xxWJgDv22Ij987xFAYgtwSXWNzwgDw
function::grep                          V4XnzOTDllwCW/e2EMuFITuX/q6stdhIQaCCOPo6ob4
function::hash                          +KkJP3kfAnYIXy1BBSEDdfzhnLpQPqz52RJpwgk+OCc
function::hook                          rhEyOmCy+ZDyO727aPXfTrEtkOOGaGKcezlkYf9BK0k
function::hooks                         2rZQUo7p9iu9gmM4wR+G6/RukNXgAk+ZcFepCgnfef8
function::identity                      6+JeD3ohDjevZF5UG7V3iRFkxIwp4KwwibpVc9dwcm4
function::import                        9HGjc6wuWn6+ElU//UxgjcG8dTo4aml/Bs117zcBdXY
function::initial-state                 E45KKMXa7YYzleuQNeMa+2XTkvSxKw40KaWN0lHLhV0
function::is                            R4wqjFaGUQnk3q8Ru9HEoyuoiTd0nEXs8655T50bO3I
function::load-state                    eLPBMJvlKzYr+6ZrhVj3kwYhpyxc9NS6veacMaV3sLk
function::lock                          rgtPo1uxq+ZA8QVPi2hstfNkbTojY7P7S4KmEwad/J4
function::ls                            qrAno4J3gk8dgDQv2/uX4Tf2Togs4RA6ijzCGuyCS4U
function::mv                            oGOOV1RR5dS32B/rTSJI2w/8FEt7LbMwDfMMZwjpqLY
function::name                          CJWkv4fyzTENKSfoZBLH4yjeaP9YkzqCiNcsULOibaE
function::parents                       PvAkixp0hXE4Q/1z7CKPil5BLkSGdGwMfg6QCGgW0EY
function::perl                          pOOr9RtVKIQI6kNIDkC1HmsIDll4TPio8OM3Cmi3htE
function::rd                            HC6cTSNHAtUtreuzLHKP6HkkDTCgI4gkzTY7t9EO//Q
function::reload                        FK+ITbNziplYOjLH/y5f4MzIJOaq/dQlHJ2CLp3+RDo
function::rm                            EwSp4uAK0uVy4lCYpJYvd0qu12q24rIlEiDUbfcW+YA
function::rmparent                      3RgK0wVGkKinmoqBJ0fZEyv72qWiL6a3unkz3etJC+M
function::save                          r7HLe7LppWXiaooOuUztnOK04LpIb+ck3bEvQ2SzY14
function::save-state                    KyoEB2NXy0ko1IYUVujXqEfbNbsoDckiKjwn8+zjvxs
function::serialize                     fpr8J5+lJ6NJGTVizsyEjq6wbTS+YecLVVnBAAkIB9k
function::serialize-single              bOoEGNxf7jcYMAbvT4n492g5aUQ9e+umiaYC574Re6E
function::sh                            PA9Q/R+AEXXCDtU8QIPNFbZr/iITh4DXEG6EWY6n4DM
function::shb                           /COiQpKD0k/4B4/fnj1q1GtH+Z+ZQV0fJUwX6kSseks
function::shell                         vmp2+XW/dSS4KbXsVcimZAzAuJloTDaQASZgG0Y07Dg
function::size                          UFrBxczp+lDNQsbGtXE7IlZULMMP2NWLqJQsUvHlr1Y
function::snapshot                      zTuPSITfomNBxxegaGuSzov/JMWJ3vUWGqAAbYx/MsY
function::snapshot-if-necessary         V2hG5mBuDkIzwasK2iNJt5v+7iK2yAYD1/Omir19AmM
function::state                         EmD3LgRcBLmEIu1loHDBwhS+F73W0KLjm9mRNws0BVI
function::touch                         MvxEZzMCnQsgc44DzkQmPpuY/gVpZjNIVGgAqznIUPE
function::u                             hxjOve/ti/MyoyBAl/1EkoXIEAB/4bOdIh8eXM3oNJs
function::unlock                        ab+C+O37Wm9wHXqiOaF+0uwd3asIzlcJcChqOpFqiwk
function::update                        67tiKQo3l50gnH1KAXcfUH6tf/tB3K64UEM2lKrcge4
function::update-from                   ZnceaN1aTEkye6SRPhI/eLnjd56gyBdAAC74YAzKkbI
function::usage                         ITBRGF9rUVDwVE+YpFc2XXNeU2tD+IJKuvUGzqp30BA
function::verify                        nRZZh97ekQOHcDQUnxrHxMhybS2RlZifwYbH56KrL3M
indicator::cc                           2ikhlSjKzneE/3j6mqjUwNQe7gyTJ6IP39JuZJXB8ow
indicator::locked                       ZtHtnMoF2VDZ6rNdzy8JHnnrO71UXJgLN+OUoiQr9Fs
indicator::path                         XVIgNhtNzG44gG2fWuZOsKqfInEUa7hFABIN1mkM6S4
internal_function::around_hook          zWPc/eIWZMVUXDctAd/u34J1WA40nlbh8SvBx2tE2VA
internal_function::associate            aRtHj4eVjuRHamUSAHaI9DVezimcdFaqFkva+dz0x7M
internal_function::attribute            T4JEqOUYjMzssdVwV/rdgAhvr0Vz9TQUo0noTdeBLxw
internal_function::attribute_is         O+KJhA7B6+N7gkDV2ajInPi947wIMEDHzAaRvWqB0a0
internal_function::cache                H0Rgkwopfv+3kVOCYBuUrh32L0pwHwBzy3LK6jhG2nU
internal_function::chmod_self           jwP33VbXCB7ZGtZ6rAm3ITeudyt+gq8wdkK/JYFvu8c
internal_function::dangerous            hpmVjFqxl2K7Kn+JMjADIOB4ZRwpsNKjulPWkUnwqng
internal_function::debug_trace          Py4lyrgkcj9TD3WxREcAOIu5qSk5ovMw/NJX/hsezMw
internal_function::execute              u5QsUyqazbIXg102zvkkR9c+U70uoAExjjTtRVss4SI
internal_function::exported             lt+j/oJ17agFjOp8QSeb2GN3j4iwEOqMulXaoAj91rI
internal_function::extension_for        VeNEZFjlo6u8rC3yvntP4xu/xpjW36Rq3PCPqlkjTiI
internal_function::fast_hash            BP8vJF0piDed/aGYR1H8SLjk9XqoGdhjpOH3wqUEFLU
internal_function::file::nuke           vRgROFbL2G7rrwziDIcamYLXWb4Sd6q325jGfqSk1f0
internal_function::file::read           ZxBqZsMZZRuLMQp8Sy//ZsoAvriDebjYLGAX7p7AxXg
internal_function::file::write          nMg2QO30cjzzI8/maAYtxQ76Ny3Tn0+67CX7xPkMu64
internal_function::fnv_hash             N2uv6uAkri1qLYjFmFOzWITS8UX0nj29kSN/Y1Z2+MM
internal_function::hypothetically       5h4k5A1NACvCeZF+auxnVnBOTfHvGYMINpFARejGqAA
internal_function::internal::main       MJVtZ4FOaoIiqG+CM+fMjVugUyU7+S6xLeirkoamEzU
internal_function::invoke_editor_on     T6uVYs1mq/VotsBwgqfn3aCoJD6JIG9+jp5M7y38ciw
internal_function::is_locked            EN0WP8v+JfHjxrEUDR5BMAJ4pMfNwj3UgG7zuj3B+yU
internal_function::metadata_from        fdIYuVCLs5te7JJdp7n2GhcSZRhZBp9O9hDdFZ3MV1s
internal_function::namespace            D7UfKyyYZ1slZZyaS28hIt8a68jkI3ELBaddROXOHug
internal_function::parent_attributes    R4t2hz0vRzfeLwj0BJWd6tIG92nhwgnJ0ghbwM9+074
internal_function::parent_ordering      jYxqyDRm6Gp0MgwofcisBAy0mFiD+TzRO4kGUo0AfW8
internal_function::retrieve             x/cC8A6IVrv7cGYc06LjgxCmn9bXFKUIWx12NXRJP8U
internal_function::retrieve_trimmed     zCzstBKNEuJKINlRSdyhF8TA812YClYWNPh6b3mhtXc
internal_function::retrieve_with_hooks  Ed7iEFKqmGXxS3JPwmygtLjlOhycYvGsGlxNt3m2vk4
internal_function::select_keys          Kpg0B01VaBHfRb1YeI3/J8KpycHwXi1c0VdLNYkagqQ
internal_function::separate_options     qvGsY0MQiEVjugLRXbi/QfHvEI+3kQr+7jDZMbapfX4
internal_function::strip                jnRBmqPwpXGicECRagXsULSsEzBuO19OHHxvE0R4zVA
internal_function::table_display        qe/73ZK9+GlMti8pCl/CPvcJYK5q5moMUiXzDgKqoJg
internal_function::temporary_name       I6v0e9aXFvCgVCeVSb84b22rwrAIOX2ywYdcr44s3Vg
internal_function::translate_backtrace  n8iPtziEsVmBboZKnIsoUXc3mbe1RLOKJFenszUeHus
internal_function::with_cwd             DCrQF2lCsAa/lbEA3i1fimbTRS//b/L4qxicIsxg+Tg
internal_function::with_exported        sFgG++rvoIvOTTIZ2QlxgblZQ7R2W9kHUn48wVvIxXI
internal_function::with_fork            fB68GdZBM7LfsOmfVsCLBv2+blwEpLgg5aXOphf7P7E
library::process                        27/7yTUHKovr2X+dF2BhebepYF81cYQA1z9Ku97kiKk
library::shell                          z9Whrvl2xc4VLK7zIYdiYnODlXqrNXY1h3TWWNip4z4
library::terminal                       Iw5HMLpx/iJYD3QzZEU7GMDAsEGK700ormCZ+biuDz0
message_color::cc                       dqUIh9jxwukwF1VCiZCtgUee4hwltDIVz1JFQeBQMmk
message_color::state                    jgobCtpCFyiG/RKX4lq/mfFDlqlACsvV8g2iAonP8C8
message_color::states                   xoWiybqyNczdKrDqkigaUhyKrzeJVJPQgAcOoA/H9dc
meta::configure                         wNvmS+fdr/xIUde0ah7ZA51enuGxvC34oW+5c+xeXIM
meta::externalize                       SAxyF6wDNTRSGW2MB+K/k5/kvjykbj5X/xtTandqrQI
meta::functor::editable                 YCK6idpseu9YEyQOH/VIRmaixSpD2Fh9/Zae85p3Z5o
meta::type::alias                       GXKpeJdADYWyo/tSo2INXijrQlakqaBtfPEHcOI5BGI
meta::type::bootstrap                   pvkTkoPACRd4InQ7QezB82EYxJJMhbvFZKPGJeN44R0
meta::type::cache                       R/GhPTCblttoS9U3XHP/UnmfQAv3pH9Q4Wt0oCKWTlc
meta::type::data                        r27u83LwuNHMNErij+j31pmG0/YiHc9Cz0JS3g0AKPA
meta::type::function                    +aTLimxYFh3zNR3AwTblpHo32WMmzet3MgkvTQNfU7I
meta::type::hook                        ivXS9PJvGTKPUaGpYgAqV2dam2DMKwIj5GFUBHstKIg
meta::type::inc                         utWgF45+/EOqDQ+Yiez7nwDBcBacz+oxdzX3G6AKxHs
meta::type::indicator                   0kZ6fU5Gw7XVjKac1A6kna6UOwjWiJB2nTtM+bWYFS8
meta::type::internal_function           i5Z/e6riR0QCY6XoP1PZNGqe29wUhWMP2iNbpMakhT0
meta::type::library                     2LKXPuOHFk8JgPJF4c/PDwpbg+jWK4/E+jeZaZd+fQY
meta::type::message_color               pxp5xSe9bUnOdskJ78dpsSeJO17phMFhVyT12w/YCGA
meta::type::meta                        E78aF6OgcaZMsgJiEo3sIPzmwen+ib/V8iFLEE2rHaU
meta::type::parent                      MH85O3I0OH6/ZemosX9G8TmHGyWVcR2TPR9LTp8WldU
meta::type::retriever                   3YDISpeCeZ0ymbeXkLIhryRyK1R7fYA8SZkQxwD1d7c
meta::type::state                       cy3e7h9soIZRo3LkUS85L+6Pz9s2pf4LKBwY7La/YII
retriever::file                         lOSZ5sIxv78zFrly1i7NEp608wPPnFvPLcAwja2WmyI
retriever::global                       RQa5dsjawOzXJqAEeMc1PWKmWZqEZYnEtbseQDph3kk
retriever::id                           uWjP6vgjXeyfpsYSJVVeS5MxmFgSAcyDWvK42znkd9c
retriever::object                       /qBb5wkKa+URfmjZ3iQFnol8IgPV9lF0UOi6PmIfLWw
retriever::perl                         DD0+Y+2YiFJ997xsJ33MrySz5MDYb2FTx9SckMnfgPI
__
internal::main();
__DATA__
