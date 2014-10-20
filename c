#!/usr/bin/perl
# 99aeabc9ec7fe80b1b39f5e53dc7e49e      <- self-modifying Perl magic
# state:  JaygV5dRfgb5Lmo2n0DIjl7EjPxCfykALlLHsS5v9qU
# istate: L3U2Fc/1v/l5imG8S1kB3Wepu+piU+KIKS6XkUoaJSc
# id:     8118b5c1b0aa08bce7e839df4ab80199

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
meta::meta('configure', <<'__');
# A function to configure transients. Transients can be used to store any number of
# different things, but one of the more common usages is type descriptors.

sub meta::configure {
  my ($datatype, %options) = @_;
  $transient{$_}{$datatype} = $options{$_} for keys %options;
}
__
meta::meta('externalize', <<'__');
# Function externalization. Data types should call this method when defining a function
# that has an external interface.

sub meta::externalize {
  my ($name, $attribute, $implementation) = @_;
  my $escaped = $name;
  $escaped =~ s/[^A-Za-z0-9:]/_/go;
  $externalized_functions{$name} = $externalized_functions{$escaped} = $attribute;
  *{"::$name"} = *{"::$escaped"} = $implementation || $attribute;
}
__
meta::meta('functor::editable', <<'__');
# An editable type. This creates a type whose default action is to open an editor
# on whichever value is mentioned. This can be changed using different flags.

sub meta::functor::editable {
  my ($typename, %options) = @_;

  meta::configure $typename, %options;
  meta::define_form $typename, sub {
    my ($name, $value) = @_;

    $options{on_bind} && &{$options{on_bind}}($name, $value);

    meta::externalize $options{prefix} . $name, "${typename}::$name", sub {
      my $attribute             = "${typename}::$name";
      my ($command, @new_value) = @_;

      return &{$options{default}}(retrieve($attribute)) if ref $options{default} eq 'CODE' and not defined $command;
      return edit($attribute) if $command eq 'edit' or $options{default} eq 'edit' and not defined $command;
      return associate($attribute, @new_value ? join(' ', @new_value) : join('', <STDIN>)) if $command eq '=' or $command eq 'import' or $options{default} eq 'import' and not defined $command;
      return retrieve($attribute)}}}
__
meta::meta('type::alias', <<'__');
meta::configure 'alias', inherit => 0, trim => 1;
meta::define_form 'alias', sub {
  my ($name, $value) = @_;
  meta::externalize $name, "alias::$name", sub {
    # Can't pre-tokenize because shell::tokenize doesn't exist until the library::
    # namespace has been evaluated (which will be after alias::).
    shell::run(shell::tokenize($value), shell::tokenize(@_));
  };
};
__
meta::meta('type::bootstrap', <<'__');
# Bootstrap attributes don't get executed. The reason for this is that because
# they are serialized directly into the header of the file (and later duplicated
# as regular data attributes), they will have already been executed when the
# file is loaded.

meta::configure 'bootstrap', extension => '.pl', inherit => 1, trim => 1;
meta::define_form 'bootstrap', sub {};
__
meta::meta('type::c', 'meta::functor::editable \'c\', extension => \'.c\', inherit => 1;');
meta::meta('type::cache', <<'__');
meta::configure 'cache', inherit => 0, trim => 1;
meta::define_form 'cache', \&meta::bootstrap::implementation;
__
meta::meta('type::cc', 'meta::functor::editable \'cc\', extension => \'.cc\', inherit => 1;');
meta::meta('type::data', 'meta::functor::editable \'data\', extension => \'\', inherit => 0, default => \'cat\';');
meta::meta('type::function', <<'__');
meta::configure 'function', extension => '.pl', inherit => 1, trim => 1;
meta::define_form 'function', sub {
  my ($name, $value) = @_;
  meta::externalize $name, "function::$name", meta::eval_in("sub {\n$value\n}", "function::$name");
};
__
meta::meta('type::h', 'meta::functor::editable \'h\', extension => \'.h\', inherit => 1;');
meta::meta('type::hh', 'meta::functor::editable \'hh\', extension => \'.hh\', inherit => 1;');
meta::meta('type::hook', <<'__');
meta::configure 'hook', extension => '.pl', inherit => 0, trim => 1;
meta::define_form 'hook', sub {
  my ($name, $value) = @_;
  *{"hook::$name"} = meta::eval_in("sub {\n$value\n}", "hook::$name");
};
__
meta::meta('type::inc', <<'__');
meta::configure 'inc', inherit => 1, extension => '.pl', trim => 1;
meta::define_form 'inc', sub {
  use File::Path 'mkpath';
  use File::Basename qw/basename dirname/;

  my ($name, $value) = @_;
  my $tmpdir   = basename($0) . '-' . $$;
  my $filename = "/tmp/$tmpdir/$name";

  push @INC, "/tmp/$tmpdir" unless grep /^\/tmp\/$tmpdir$/, @INC;

  mkpath(dirname($filename));
  unless (-e $filename) {
    open my $fh, '>', $filename;
    print $fh $value;
    close $fh;
  }
};
__
meta::meta('type::indicator', <<'__');
# Shell indicator function. The output of each of these is automatically
# appended to the shell prompt.

meta::configure 'indicator', inherit => 1, extension => '.pl', trim => 1;
meta::define_form 'indicator', sub {
  my ($name, $value) = @_;
  *{"indicator::$name"} = meta::eval_in("sub {\n$value\n}", "indicator::$name");
};
__
meta::meta('type::internal_function', <<'__');
meta::configure 'internal_function', extension => '.pl', inherit => 1, trim => 1;
meta::define_form 'internal_function', sub {
  my ($name, $value) = @_;
  *{$name} = meta::eval_in("sub {\n$value\n}", "internal_function::$name");
};
__
meta::meta('type::library', <<'__');
meta::configure 'library', extension => '.pl', inherit => 1, trim => 1;
meta::define_form 'library', sub {
  my ($name, $value) = @_;
  meta::eval_in($value, "library::$name");
};
__
meta::meta('type::message_color', <<'__');
meta::configure 'message_color', extension => '', inherit => 1, trim => 1;
meta::define_form 'message_color', sub {
  my ($name, $value) = @_;
  terminal::color($name, $value);
};
__
meta::meta('type::meta', <<'__');
# This doesn't define a new type. It customizes the existing 'meta' type
# defined in bootstrap::initialization. Note that horrible things will
# happen if you redefine it using the editable functor.

meta::configure 'meta', extension => '.pl', inherit => 1, trim => 1;
__
meta::meta('type::parent', <<'__');
meta::define_form 'parent', \&meta::bootstrap::implementation;
meta::configure 'parent', extension => '', inherit => 1, trim => 1;
__
meta::meta('type::retriever', <<'__');
meta::configure 'retriever', extension => '.pl', inherit => 1, trim => 1;
meta::define_form 'retriever', sub {
  my ($name, $value) = @_;
  $transient{retrievers}{$name} = meta::eval_in("sub {\n$value\n}", "retriever::$name");
};
__
meta::meta('type::state', <<'__');
# Allows temporary or long-term storage of states. Nothing particularly insightful
# is done about compression, so storing alternative states will cause a large
# increase in size. Also, states don't contain other states -- otherwise the size
# increase would be exponential.

# States are created with the save-state function.

meta::configure 'state', inherit => 0, extension => '.pl', trim => 1;
meta::define_form 'state', \&meta::bootstrap::implementation;
__
meta::bootstrap('html', <<'__');
<html>
  <head>
  <meta http-equiv='content-type' content='text/html; charset=UTF-8' />
  <link rel='stylesheet' href='http://spencertipping.com/perl-objects/web/style.css'/>

  <script src='http://ajax.googleapis.com/ajax/libs/jquery/1.5.2/jquery.min.js'></script>
  <script src='http://spencertipping.com/caterwaul/caterwaul.all.min.js'></script>
  <script src='http://spencertipping.com/montenegro/montenegro.client.js'></script>
  <script src='http://spencertipping.com/perl-objects/web/attribute-parser.js'></script>
  <script src='http://spencertipping.com/perl-objects/web/interface.js'></script>
  </head>
  <body></body>
</html>
__
meta::bootstrap('initialization', <<'__');
#!/usr/bin/perl
# 99aeabc9ec7fe80b1b39f5e53dc7e49e      <- self-modifying Perl magic
# state:  __state
# istate: __istate
# id:     __id

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
__
meta::bootstrap('perldoc', <<'__');
=head1 Self-modifying Perl script

=head2 Original implementation by Spencer Tipping L<http://spencertipping.com>

The prototype for this script is licensed under the terms of the MIT source code license.
However, this script in particular may be under different licensing terms. To find out how
this script is licensed, please contact whoever sent it to you. Alternatively, you may
run it with the 'license' argument if they have specified a license that way.

You should not edit this file directly. For information about how it was constructed, go
to L<http://spencertipping.com/writing-self-modifying-perl>. For quick usage guidelines,
run this script with the 'usage' argument.

=cut
__
meta::cache('parent-identification', 'object 99aeabc9ec7fe80b1b39f5e53dc7e49e');
meta::cache('parent-state', '99aeabc9ec7fe80b1b39f5e53dc7e49e vn6IaB8HbqRG8OhD2TkFAh93pW/sYPP6EH7zaFuPda0');
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
meta::data('permanent-identity', '8118b5c1b0aa08bce7e839df4ab80199');
meta::function('ad', <<'__');
my ($options, @paths) = separate_options(@_);
@{$transient{path}} = () if $$options{-c};
return @{$transient{path}} = () unless @paths;
push @{$transient{path}}, @paths;
__
meta::function('alias', <<'__');
my ($name, @stuff) = @_;
@_ ? @stuff ? around_hook('alias', @_, sub {associate("alias::$name", join(' ', @stuff), execute => 1)})
            : retrieve("alias::$name") // "Undefined alias $name"
   : table_display([select_keys('--namespace' => 'alias')], [map retrieve($_), select_keys('--namespace' => 'alias')]);
__
meta::function('cat', 'join "\\n", retrieve(@_);');
meta::function('cc', <<'__');
# Stashes a quick one-line continuation. (Used to remind me what I was doing.)
@_ ? associate('data::current-continuation', hook('set-cc', join(' ', @_))) : retrieve('data::current-continuation');
__
meta::function('ccc', 'rm(\'data::current-continuation\');');
meta::function('cd', <<'__');
my ($options, @stuff) = separate_options(@_);
my @new_stuff = map "${_}::", @stuff;
ad(%$options, @new_stuff);
__
meta::function('child', <<'__');
around_hook('child', @_, sub {
  my ($child_name) = @_;
  clone($child_name);
  enable();
  qx($child_name update-from $0);
  terminal::info("$child_name\'s identity is " . join '', qx($child_name identity));
  disable()});
__
meta::function('clone', <<'__');
my ($options, @files) = separate_options(@_);
for my $file (@files) {
  around_hook('clone', $file, sub {
    hypothetically(sub {
      # Assign a new object identity.
      rm('data::permanent-identity');
      identity();

      file::write($file, serialize(), noclobber => 1);
      chmod(0700, $file)})})}
__
meta::function('cp', <<'__');
my $from = shift @_;
my $value = retrieve($from);
associate($_, $value) for @_;
__
meta::function('create', <<'__');
my ($name, $value) = @_;
around_hook('create', $name, $value, sub {
  return edit($name) if exists $data{$name};
  associate($name, defined $value ? $value : '');
  edit($name) unless defined $value});
__
meta::function('current-state', 'serialize(\'-pS\');');
meta::function('cwd', <<'__');
use Cwd qw/getcwd/;
getcwd();
__
meta::function('disable', 'hook(\'disable\', chmod_self(sub {$_[0] & 0666}));');
meta::function('edit', <<'__');
my ($options, @names) = separate_options(@_);
@names = select_keys('--criteria' => "^$$options{'--prefix'}.*" . join('.*', @names), %$options) if $$options{'--prefix'};

my ($name, @others) = @names;
die "cannot edit multiple attributes simultaneously (others are @others)" if @others;
die "$name is virtual or does not exist" unless exists $data{$name};
die "$name is inherited; use 'edit $name -f' to edit anyway" unless is($name, '-u') || is($name, '-d') || exists $$options{'-f'};

my $extension = extension_for($name);
my $changed   = 0;
around_hook('edit', @_, sub {
  my $result = invoke_editor_on($data{$name} // '', %$options, attribute => $name, extension => $extension);
  $changed += $data{$name} ne $result;
  associate($name, $result, execute => 1)});

save() unless !$changed or $data{'data::edit::no-save'} or state() eq $transient{initial};
$changed;
__
meta::function('edit-self', <<'__');
$global_data = invoke_editor_on($global_data);
save();
__
meta::function('enable', 'hook(\'enable\', chmod_self(sub {$_[0] | ($_[0] & 0444) >> 2}));');
meta::function('expanded-bootstrap', <<'__');
# Write headers into the bootstrap section. There is some subtle stuff going on
# here with the istate header. The idea is to provide other objects a very quick
# way to see whether our state has changed, but we don't want false positives.
# We would get a false positive if, for instance, we included the contents of
# parent:: attributes in the istate hash. The reason is that the parent::
# attribute contains a hash of every attribute provided by that parent, so any
# change in the parent would impact the istate of the child.
#
# The best way to deal with this is to treat parent:: attributes as being
# opaque; we record their existence or nonexistence, but we don't record their
# contents. We also look only at inheritable and unique attributes and fail to
# consider global state.

my $bootstrap_text = retrieve('bootstrap::initialization');
my $state          = state();
my $istate         = state('-iGP');
my $object_id      = identity();

$bootstrap_text =~ s/__state/$state/g;
$bootstrap_text =~ s/__istate/$istate/g;
$bootstrap_text =~ s/__id/$object_id/g;

$bootstrap_text;
__
meta::function('export', <<'__');
# Exports data into a text file.
#   export attr1 attr2 attr3 ... file.txt
my $name = pop @_;
@_ or die 'Expected filename';
file::write($name, join "\n", retrieve(@_));
__
meta::function('extern', '&{$_[0]}(retrieve(@_[1 .. $#_]));');
meta::function('grep', <<'__');
# Looks through attributes for a pattern. Usage is grep pattern [options], where
# [options] is the format as provided to select_keys.

my ($pattern, @args)     = @_;
my ($options, @criteria) = separate_options(@args);
my @attributes           = select_keys(%$options, '--criteria' => join('|', @criteria));

$pattern = qr/$pattern/;

my @m_attributes;
my @m_line_numbers;
my @m_lines;

for my $k (@attributes) {
  next unless length $k;
  my @lines = split /\n/, retrieve($k);
  for (0 .. $#lines) {
    next unless $lines[$_] =~ $pattern;
    push @m_attributes,   $k;
    push @m_line_numbers, $_ + 1;
    push @m_lines,        '' . ($lines[$_] // '')}}

unless ($$options{'-C'}) {
  s/($pattern)/\033[1;31m\1\033[0;0m/g for @m_lines;
  s/^/\033[1;34m/o for @m_attributes;
  s/^/\033[1;32m/o && s/$/\033[0;0m/o for @m_line_numbers}

table_display([@m_attributes], [@m_line_numbers], [@m_lines]);
__
meta::function('hash', 'fast_hash(@_);');
meta::function('hook', <<'__');
my ($hook, @args) = @_;
$transient{active_hooks}{$hook} = 1;
dangerous('', sub {&$_(@args)}) for grep /^hook::${hook}::/, sort keys %data;
@args;
__
meta::function('hooks', 'join "\\n", sort keys %{$transient{active_hooks}};');
meta::function('identity', <<'__');
retrieve('data::permanent-identity') or
associate('data::permanent-identity', fast_hash(join '|', map rand(), 1 .. 32));
__
meta::function('import', <<'__');
my $name = pop @_;
associate($name, @_ ? join('', map(file::read($_), @_)) : join('', <STDIN>));
__
meta::function('initial-state', '$transient{initial};');
meta::function('is', <<'__');
my ($attribute, @criteria) = @_;
my ($options, @stuff) = separate_options(@criteria);
exists $data{$attribute} and attribute_is($attribute, %$options);
__
meta::function('journal', <<'__');
# Writes a Perl file that reproduces the current object, reusing parents.
my @parents = grep s/^parent:://, sort keys %data;
my @pieces  =
  ('#!/usr/bin/env perl',
   "# This file was generated by running '$0 journal'",
   q[die "usage: $0 output-file (where output-file doesn't exist yet)"],
   q[  unless @ARGV == 1 && ! -e $ARGV[0];]);

# Special case: if no parents, then we need to emit the whole object.
# Technically, this should happen only for the 'object' script, but maybe you
# cloned it.
unless (@parents) {
  my $hash = state();
  push @pieces, q[my $output = $ARGV[0];],
                q[open my $fh, "> $output" ],
                q[  or die "failed to open output file $ARGV[0]: $!";],
                "\$fh->print <<'$hash';",
                serialize(),
                $hash,
                "close \$fh;",
                "chmod 0700, \$output;";
} else {
  # Otherwise, start by constructing an object from one of the parents and
  # inherit from the others.
  my ($primary, @others) = @parents;
  push @pieces,  q[my $output = $ARGV[0];],
                qq[system '$primary', 'child', \$output],
                qq[  and die "failed to create object from $primary";],
                map {; qq[system 'perl', \$output, 'update-from', '$_', '-SV'],
                       qq[  and die "failed to update from $_";]}
                    @others;

  # Now reconstruct divergent and/or uninherited attributes.
  for my $divergent (select_keys('-d' => 1), select_keys('-u' => 1)) {
    my $s = serialize_single($divergent) =~ s/^[^(]+\('[^']+',\s*/\$fh->print(/r;
    push @pieces, qq[open my \$fh, "| perl \$output import $divergent"],
                  qq[  or die "failed to create attribute $divergent";],
                  qq[$s],
                  qq[close \$fh;];
  }

  push @pieces, "chmod 0700, \$output;";
}

join "\n", @pieces;
__
meta::function('load-state', <<'__');
around_hook('load-state', @_, sub {
  my ($state_name) = @_;
  my $state = retrieve("state::$state_name");

  terminal::state('saving current state into _...');
  save_state('_');

  delete $data{$_} for grep ! /^state::/, keys %data;
  %externalized_functions = ();

  terminal::state("restoring state $state_name...");
  meta::eval_in($state, "state::$state_name");
  terminal::error(hook('load-state-failed', $@)) if $@;
  reload();
  verify()});
__
meta::function('lock', 'hook(\'lock\', chmod_self(sub {$_[0] & 0555}));');
meta::function('ls', <<'__');
my ($options, @criteria) = separate_options(@_);
my ($external, $shadows, $sizes, $flags, $long, $hashes, $parent_hashes) = @$options{qw(-e -s -z -f -l -h -p)};
$sizes = $flags = $hashes = $parent_hashes = 1 if $long;

return table_display([grep ! exists $data{$externalized_functions{$_}}, sort keys %externalized_functions]) if $shadows;

my $criteria    = join('|', @criteria);
my @definitions = select_keys('--criteria' => $criteria, '--path' => $transient{path}, %$options);

my %inverses  = map {$externalized_functions{$_} => $_} keys %externalized_functions;
my @externals = map $inverses{$_}, grep length, @definitions;
my @internals = grep length $inverses{$_}, @definitions;
my @sizes     = map sprintf('%6d %6d', length(serialize_single($_)), length(retrieve($_))), @{$external ? \@internals : \@definitions} if $sizes;

my @flags     = map {my $k = $_; join '', map(is($k, "-$_") ? $_ : '-', qw(d i m u))} @definitions if $flags;
my @hashes    = map fast_hash(retrieve($_)), @definitions if $hashes;

my %inherited     = parent_attributes(grep /^parent::/o, keys %data) if $parent_hashes;
my @parent_hashes = map $inherited{$_} || '-', @definitions if $parent_hashes;

join "\n", map strip($_), split /\n/, table_display($external ? [grep length, @externals] : [@definitions],
                                                    $sizes ? ([@sizes]) : (), $flags ? ([@flags]) : (), $hashes ? ([@hashes]) : (), $parent_hashes ? ([@parent_hashes]) : ());
__
meta::function('mv', <<'__');
my ($from, $to) = @_;
die "'$from' does not exist" unless exists $data{$from};
associate($to, retrieve($from), execute => 1);
rm($from);
__
meta::function('name', <<'__');
my $name = $0;
$name =~ s/^.*\///;
$name;
__
meta::function('parents', 'join "\\n", grep s/^parent:://o, sort keys %data;');
meta::function('perl', <<'__');
my @result = eval(join ' ', @_);
$@ ? terminal::error($@) : wantarray ? @result : $result[0];
__
meta::function('rd', <<'__');
if (@_) {my $pattern = join '|', @_;
         @{$transient{path}} = grep $_ !~ /^$pattern$/, @{$transient{path}}}
else    {pop @{$transient{path}}}
__
meta::function('reload', 'around_hook(\'reload\', sub {execute($_) for grep ! /^bootstrap::/, keys %data});');
meta::function('rm', <<'__');
around_hook('rm', @_, sub {
  exists $data{$_} or terminal::warning("$_ does not exist") for @_;
  delete @data{@_}});
__
meta::function('rmparent', <<'__');
# Removes one or more parents.
my ($options, @parents) = separate_options(@_);
my $clobber_divergent = $$options{'-D'} || $$options{'--clobber-divergent'};

my %parents = map {$_ => 1} @parents;
my @other_parents = grep !$parents{$_}, grep s/^parent:://, select_keys('--namespace' => 'parent');
my %kept_by_another_parent;

$kept_by_another_parent{$_} = 1 for grep s/^(\S+)\s.*$/\1/, split /\n/o, cat(@other_parents);

for my $parent (@parents) {
  my $keep_parent_around = 0;

  for my $line (split /\n/, retrieve("parent::$parent")) {
    my ($name, $hash) = split /\s+/, $line;
    next unless exists $data{$name};

    my $local_hash = fast_hash(retrieve($name));
    if ($clobber_divergent or $hash eq $local_hash or ! defined $hash) {rm($name) unless $kept_by_another_parent{$name}}
    else {terminal::info("local attribute $name exists and is divergent; use rmparent -D $parent to delete it");
          $keep_parent_around = 1}}

  $keep_parent_around ? terminal::info("not deleting parent::$parent so that you can run", "rmparent -D $parent if you want to nuke divergent attributes too")
                      : rm("parent::$parent")}
__
meta::function('save', 'around_hook(\'save\', sub {dangerous(\'\', sub {file::write($0, serialize(\'-V\')); $transient{initial} = state()}) if verify()});');
meta::function('save-state', <<'__');
# Creates a named copy of the current state and stores it.
my ($state_name) = @_;
around_hook('save-state', $state_name, sub {
  associate("state::$state_name", current_state(), execute => 1)});
__
meta::function('serialize', <<'__');
my ($options, @criteria) = separate_options(@_);
delete $$options{'-P'};

my $partial     = delete $$options{'-p'};
my $criteria    = join '|', @criteria;
my @attributes  = map serialize_single($_), select_keys(%$options, '-m' => 1, '--criteria' => $criteria), select_keys(%$options, '-M' => 1, '--criteria' => $criteria);
my @final_array = @{$partial ? \@attributes : [expanded_bootstrap(), @attributes, 'internal::main();', '', '__DATA__', $global_data]};
join "\n", @final_array;
__
meta::function('serialize-single', <<'__');
# Serializes a single attribute and optimizes for content.

my $name          = $_[0] || $_;
my $contents      = retrieve_trimmed($name);
my $meta_function = 'meta::' . namespace($name);
my $invocation    = attribute($name);

if ($contents !~ /\v/) {
  $contents =~ s/\\/\\\\/go;
  $contents =~ s/'/\\'/go;
  return "$meta_function('$invocation', '$contents');"}

my $delimiter = '__' . fast_hash($contents);
my $chars     = 2;

++$chars until $chars >= length($delimiter) || index("\n$contents", "\n" . substr($delimiter, 0, $chars)) == -1;
$delimiter = substr($delimiter, 0, $chars);

"$meta_function('$invocation', <<'$delimiter');\n$contents\n$delimiter";
__
meta::function('sh', <<'__');
around_hook('sh', @_, sub {
  system(@_)});
__
meta::function('shb', <<'__');
# Backgrounded shell.
with_fork(@_, \&::sh);
__
meta::function('shell', <<'__');
my ($options, @arguments) = separate_options(@_);
$transient{repl_prefix} = $$options{'--repl-prefix'};

terminal::cc(retrieve('data::current-continuation')) if length $data{'data::current-continuation'};
around_hook('shell', sub {shell::repl(%$options)});
__
meta::function('size', <<'__');
my $size = 0;
$size += length $data{$_} for keys %data;
sprintf "   full logical  unique    self\n% 7d % 7d % 7d % 7d", length(serialize()), $size, length(serialize('-up')), length $global_data;
__
meta::function('snapshot', <<'__');
my ($name) = @_;
file::write(my $finalname = temporary_name($name), serialize(), noclobber => 1);
chmod 0700, $finalname;
hook('snapshot', $finalname);
__
meta::function('snapshot-if-necessary', 'snapshot() if state() ne $transient{initial};');
meta::function('state', <<'__');
my ($options, @attributes) = separate_options(@_);
@attributes = grep !is($_, '-v'), sort keys %data unless @attributes;
@attributes = grep is($_, '-iu'), @attributes if $$options{'-i'};
@attributes = grep is($_, '-P'),  @attributes if $$options{'-P'};

my $hash = fast_hash(fast_hash(scalar @attributes) . join '|', @attributes);
$hash = fast_hash(retrieve_trimmed($_) . "|$hash") for @attributes;

$hash = fast_hash(join '|', $hash, grep s/^parent:://, sort keys %data)
if $$options{'-P'};

$$options{'-G'} ? $hash : fast_hash("$global_data|$hash");
__
meta::function('touch', 'associate($_, \'\') for @_;');
meta::function('u', <<'__');
my ($options, $count) = separate_options(@_);
$count //= 1;
rd() while $count--;
__
meta::function('unlock', 'hook(\'unlock\', chmod_self(sub {$_[0] | 0200}));');
meta::function('update', 'update_from(@_, grep s/^parent:://o, sort keys %data);');
meta::function('update-from', <<'__');
# Upgrade all attributes that aren't customized. Customization is defined when the data type is created,
# and we determine it here by checking for $transient{inherit}{$type}.

# Note that this assumes you trust the remote script. If you don't, then you shouldn't update from it.

around_hook('update-from-invocation', separate_options(@_), sub {
  my ($options, @targets) = @_;
  my %parent_id_cache    = cache('parent-identification');
  my %parent_state_cache = cache('parent-state');
  my %already_seen;

  @targets or return;

  my @known_targets     = grep s/^parent:://, parent_ordering(map "parent::$_", grep exists $data{"parent::$_"}, @targets);
  my @unknown_targets   = grep ! exists $data{"parent::$_"}, @targets;
  @targets = (@known_targets, @unknown_targets);

  my $save_state        = $$options{'-s'} || $$options{'--save'};
  my $no_state          = $$options{'-S'} || $$options{'--no-state'};
  my $no_verify         = $$options{'-V'} || $$options{'--no-verify'};
  my $no_parents        = $$options{'-P'} || $$options{'--no-parent'} || $$options{'--no-parents'};
  my $force             = $$options{'-f'} || $$options{'--force'};
  my $clobber_divergent = $$options{'-D'} || $$options{'--clobber-divergent'};

  my $can_skip_already_seen = !($$options{'-K'} || $$options{'--no-skip'}) &&
                              !$force && !$clobber_divergent;

  save_state('before-update') unless $no_state;

  for my $target (@targets) {
    dangerous("updating from $target", sub {
    around_hook('update-from', $target, sub {
      my $target_filename = strip(qx(which $target)) || $target;
      my %parent_metadata = %{metadata_from($target_filename)};
      terminal::warning("$target_filename has no externally visible metadata (makes updating slower)") unless $parent_metadata{id};

      my $identity = $parent_id_cache{$target} ||= $parent_metadata{id} || join '', qx($target identity);
      next if $can_skip_already_seen and
              exists $data{"parent::$target"} and
              $already_seen{$identity} || $parent_state_cache{$identity} eq $parent_metadata{istate};

      my $attributes = join '', qx($target ls -ahiu);
      my %divergent;
      die "skipping unreachable $target" unless $attributes;

      # These need to come after the reachability check so that we retry against
      # other copies in case something fails.
      ++$already_seen{$identity};
      $parent_state_cache{$identity} = $parent_metadata{istate} || join '', qx($target state -iPG);

      for my $to_rm (split /\n/, retrieve("parent::$target")) {
        my ($name, $hash) = split(/\s+/, $to_rm);
        next unless exists $data{$name};

        my $local_hash = fast_hash(retrieve($name));
        if ($clobber_divergent or $hash eq $local_hash or ! defined $hash) {rm($name)}
        else {terminal::info("preserving local version of divergent attribute $name (use update -D to clobber it)");
              $divergent{$name} = retrieve($name)}}

      associate("parent::$target", $attributes) unless $no_parents;

      dangerous('', sub {eval qx($target serialize -ipmu)});
      dangerous('', sub {eval qx($target serialize -ipMu)});

      map associate($_, $divergent{$_}), keys %divergent unless $clobber_divergent;

      reload()})})}

  cache('parent-identification', %parent_id_cache);
  cache('parent-state',          %parent_state_cache);

  if ($no_verify) {hook('update-from-presumably-succeeded', $options, @targets);
                   rm('state::before-update') unless $no_state || $save_state}
  elsif (verify()) {hook('update-from-succeeded', $options, @targets);
                    terminal::info("Successfully updated. Run 'load-state before-update' to undo this change.") if $save_state;
                    rm('state::before-update') unless $no_state || $save_state}
  elsif ($force || $no_state) {hook('update-from-failed', $options, @targets);
                               terminal::warning('Failed to verify: at this point your object will not save properly, though backup copies will be created.',
                                                 $no_state ? 'You should attempt to repair this object since no prior state was saved.'
                                                           : 'Run "load-state before-update" to undo the update and return to a working state.')}
  else {hook('update-from-failed', $options, @targets);
        terminal::error('Verification failed after the upgrade was complete.');
        terminal::info("$0 has been reverted to its pre-upgrade state.", "If you want to upgrade and keep the failure state, then run 'update-from $target --force'.");
        load_state('before-update');
        rm('state::before-update')}});
__
meta::function('usage', '"Usage: $0 action [arguments]\\nUnique actions (run \'$0 ls\' to see all actions):" . ls(\'-u\');');
meta::function('verify', <<'__');
file::write(my $other = $transient{temporary_filename} = temporary_name(), my $serialized_data = serialize());
chomp(my $observed = join '', qx|perl '$other' state|);

unlink $other if my $result = $observed eq (my $state = state());
terminal::error("Verification failed; expected $state but got $observed from $other") unless $result;
hook('after-verify', $result, observed => $observed, expected => $state);
$result;
__
meta::indicator('cc', 'length ::retrieve(\'data::current-continuation\') ? "\\033[1;36mcc\\033[0;0m" : \'\';');
meta::indicator('locked', 'is_locked() ? "\\033[1;31mlocked\\033[0;0m" : \'\';');
meta::indicator('path', <<'__');
my @highlighted = map join("\033[1;30m|\033[0;0m", split /\|/, $_), @{$transient{path}};
join "\033[1;30m/\033[0;0m", @highlighted;
__
meta::internal_function('around_hook', <<'__');
# around_hook('hookname', @args, sub {
#   stuff;
# });

# Invokes 'before-hookname' on @args before the sub runs, invokes the
# sub on @args, then invokes 'after-hookname' on @args afterwards.
# The after-hook is not invoked if the sub calls 'die' or otherwise
# unwinds the stack.

my $hook = shift @_;
my $f    = pop @_;

hook("before-$hook", @_);
my @result = &$f(@_);
hook("after-$hook", @_, @result);
wantarray ? @result : $result[0];
__
meta::internal_function('associate', <<'__');
my ($name, $value, %options) = @_;
die "Namespace does not exist" unless exists $datatypes{namespace($name)};
$data{$name} = $value;
execute($name) if $options{execute};
$value;
__
meta::internal_function('attribute', <<'__');
my ($name) = @_;
$name =~ s/^[^:]*:://;
$name;
__
meta::internal_function('attribute_is', <<'__');
my ($a, %options) = @_;
my %inherited     = parent_attributes(grep /^parent::/o, sort keys %data) if grep exists $options{$_}, qw/-u -U -d -D/;
my $criteria      = $options{'--criteria'} || $options{'--namespace'} && "^$options{'--namespace'}::" || '.';

my %tests = ('-u' => sub {! $inherited{$a}},
             '-d' => sub {$inherited{$a} && fast_hash(retrieve($a)) ne $inherited{$a}},
             '-i' => sub {$transient{inherit}{namespace($a)}},
             '-v' => sub {$transient{virtual}{namespace($a)}},
             '-p' => sub {$a =~ /^parent::/o},
             '-s' => sub {$a =~ /^state::/o},
             '-m' => sub {$a =~ /^meta::/o});

return 0 unless scalar keys %tests == scalar grep ! exists $options{$_}    ||   &{$tests{$_}}(), keys %tests;
return 0 unless scalar keys %tests == scalar grep ! exists $options{uc $_} || ! &{$tests{$_}}(), keys %tests;

$a =~ /$_/ || return 0 for @{$options{'--path'}};
$a =~ /$criteria/;
__
meta::internal_function('cache', <<'__');
my ($name, %pairs) = @_;
if (%pairs) {associate("cache::$name", join "\n", map {$pairs{$_} =~ s/\n//g; "$_ $pairs{$_}"} sort keys %pairs)}
else        {map split(/\s/, $_, 2), split /\n/, retrieve("cache::$name")}
__
meta::internal_function('chmod_self', <<'__');
my ($mode_function)      = @_;
my (undef, undef, $mode) = stat $0;
chmod &$mode_function($mode), $0;
__
meta::internal_function('dangerous', <<'__');
# Wraps a computation that may produce an error.
my ($message, $computation) = @_;
terminal::info($message) if $message;
my @result = eval {&$computation()};
terminal::warning(translate_backtrace($@)), return undef if $@;
wantarray ? @result : $result[0];
__
meta::internal_function('debug_trace', <<'__');
terminal::debug(join ', ', @_);
wantarray ? @_ : $_[0];
__
meta::internal_function('execute', <<'__');
my ($name, %options) = @_;
my $namespace = namespace($name);
eval {&{$datatypes{$namespace}}(attribute($name), retrieve($name))};
warn $@ if $@ && $options{'carp'};
__
meta::internal_function('exported', <<'__');
# Allocates a temporary file containing the concatenation of attributes you specify,
# and returns the filename. The filename will be safe for deletion anytime.
my $filename = temporary_name();
file::write($filename, cat(@_));
$filename;
__
meta::internal_function('extension_for', <<'__');
my $extension = $transient{extension}{namespace($_[0])};
$extension = &$extension($_[0]) if ref $extension eq 'CODE';
$extension || '';
__
meta::internal_function('fast_hash', <<'__');
use Digest::SHA qw/sha256_base64/;
sha256_base64(@_);
__
meta::internal_function('file::nuke', <<'__');
# Nukes a file with the preferred mechanism.
my ($file)  = @_;
my $backend = eval {file_nuke_backend()} // 'shred';
return system("shred -u '$file'") if $backend eq 'shred';
return unlink $file if $backend eq 'unlink';
die "unknown file::nuke backend: $backend";
__
meta::internal_function('file::read', <<'__');
my $name = shift;
open my($handle), "<", $name;
my $result = join "", <$handle>;
close $handle;
$result;
__
meta::internal_function('file::write', <<'__');
use File::Path     'mkpath';
use File::Basename 'dirname';

my ($name, $contents, %options) = @_;
die "Choosing not to overwrite file $name" if $options{noclobber} and -f $name;
mkpath(dirname($name)) if $options{mkpath};

my $open_name = $name =~ /^[>|]/ ? $name : $options{append} ? ">> $name" : "> $name";
open my($handle), $open_name or die "Can't open $name for writing";
print $handle $contents;
close $handle;
__
meta::internal_function('fnv_hash', <<'__');
# A rough approximation to the Fowler-No Voll hash. It's been 32-bit vectorized
# for efficiency, which may compromise its effectiveness for short strings.

my ($data) = @_;

my ($fnv_prime, $fnv_offset) = (16777619, 2166136261);
my $hash                     = $fnv_offset;
my $modulus                  = 2 ** 32;

$hash = ($hash ^ ($_ & 0xffff) ^ ($_ >> 16)) * $fnv_prime % $modulus for unpack 'L*', $data . substr($data, -4) x 8;
$hash;
__
meta::internal_function('hypothetically', <<'__');
# Allows you to make changes to the data without committing them.
# Usage:
#
# hypothetically(sub {
#   ...
# });
#
# Changes to %data made inside the sub {} are discarded.

my %data_backup   = %data;
my ($side_effect) = @_;
my @result        = eval {&$side_effect()};
%data = %data_backup;

die $@ if $@;
wantarray ? @result : $result[0];
__
meta::internal_function('internal::main', <<'__');
disable();

$SIG{'INT'}              = sub {snapshot_if_necessary(); exit 1};
$transient{initial}      = state();
chomp(my $default_action = retrieve('data::default-action'));

my $function_name = shift(@ARGV) || $default_action;
my @effective_argv = @ARGV;

unshift @effective_argv, $function_name and $function_name = 'method_missing'
unless exists $externalized_functions{$function_name};

around_hook('main-function', $function_name, @effective_argv, sub {
  dangerous('', sub {
    chomp(my @result = &$function_name(@effective_argv));
    print join("\n", @result), "\n" if @result})});

save() unless state() eq $transient{initial};

END {enable()}
__
meta::internal_function('invoke_editor_on', <<'__');
my ($data, %options) = @_;
my $editor    = $options{editor} || $ENV{VISUAL} || $ENV{EDITOR} || die 'Either the $VISUAL or $EDITOR environment variable should be set to a valid editor';
my $options   = $options{options} || $ENV{VISUAL_OPTS} || $ENV{EDITOR_OPTS} || '';
my $attribute = $options{attribute};
$attribute =~ s/\//-/g;
my $filename  = temporary_name() . "-$attribute$options{extension}";

file::write($filename, $data);
system("$editor $options '$filename'");

my $result = file::read($filename);
file::nuke($filename);
$result;
__
meta::internal_function('is_locked', '!((stat($0))[2] & 0222);');
meta::internal_function('metadata_from', <<'__');
my ($filename) = @_;
my %metadata;

# Not using file::read because we only need the first few lines.
open my($fh), '<', $filename or return {};
while (<$fh>) {
  $metadata{$1} = $2 if /^#\s*(\w+):\s*(.*)$/;
  last unless /^#/;
}
close $fh;

\%metadata;
__
meta::internal_function('namespace', <<'__');
my ($name) = @_;
$name =~ s/::.*$//;
$name;
__
meta::internal_function('parent_attributes', <<'__');
my $attributes = sub {my ($name, $value) = split /\s+/o, $_; $name => ($value || 1)};
map &$attributes(), split /\n/o, join("\n", retrieve(@_));
__
meta::internal_function('parent_ordering', <<'__');
# Topsorts the parents by dependency chain. The simplest way to do this is to
# transitively compute the number of parents referred to by each parent.

my @parents = @_;
my %all_parents = map {$_ => 1} @parents;

my %parents_of = map {
  my $t = $_;
  my %attributes = parent_attributes($_);
  $t => [grep /^parent::/, keys %attributes]} @parents;

my %parent_count;
my $parent_count;
$parent_count = sub {
  my ($key) = @_;
  return $parent_count{$key} if exists $parent_count{$key};
  my $count = 0;
  $count += $parent_count->($_) + exists $data{$_} for @{$parents_of{$key}};
  $parent_count{$key} = $count};

my %inverses;
push @{$inverses{$parent_count->($_)} ||= []}, $_ for @parents;
grep exists $all_parents{$_}, map @{$inverses{$_}}, sort keys %inverses;
__
meta::internal_function('retrieve', <<'__');
my @results = map defined $data{$_} ? $data{$_} : retrieve_with_hooks($_), @_;
wantarray ? @results : $results[0];
__
meta::internal_function('retrieve_trimmed', <<'__');
return strip(retrieve($_[0])) if $transient{trim}{namespace($_[0])};
retrieve($_[0]);
__
meta::internal_function('retrieve_with_hooks', <<'__');
# Uses the hooks defined in $transient{retrievers}, and returns undef if none work.
my ($attribute) = @_;
my $result      = undef;

defined($result = &$_($attribute)) and return $result for map $transient{retrievers}{$_}, sort keys %{$transient{retrievers}};
return undef;
__
meta::internal_function('select_keys', <<'__');
my %options = @_;
grep attribute_is($_, %options), sort keys %data;
__
meta::internal_function('separate_options', <<'__');
# Things with one dash are short-form options, two dashes are long-form.
# Characters after short-form are combined; so -auv4 becomes -a -u -v -4.
# Also finds equivalences; so --foo=bar separates into $$options{'--foo'} eq 'bar'.
# Stops processing at the -- option, and removes it. Everything after that
# is considered to be an 'other' argument.

# The only form not supported by this function is the short-form with argument.
# To pass keyed arguments, you need to use long-form options.

my @parseable;
push @parseable, shift @_ until ! @_ or $_[0] eq '--';

my @singles = grep /^-[^-]/, @parseable;
my @longs   = grep /^--/,    @parseable;
my @others  = grep ! /^-/,   @parseable;

my @singles = map /-(.{2,})/ ? map("-$_", split(//, $1)) : $_, @singles;

my %options;
/^([^=]+)=(.*)$/ and $options{$1} = $2 for @longs;
++$options{$_} for grep ! /=/, @singles, @longs;

({%options}, grep length, @others, @_);
__
meta::internal_function('strip', 'wantarray ? map {s/^\\s*|\\s*$//sgo; $_} @_ : $_[0] =~ /^\\s*(.*?)\\s*$/so && $1;');
meta::internal_function('table_display', <<'__');
# Displays an array of arrays as a table; that is, with alignment. Arrays are
# expected to be in column-major order.

sub maximum_length_in {
  my $maximum = 0;
  length > $maximum and $maximum = length for @_;
  $maximum;
}

my @arrays    = @_;
my @lengths   = map maximum_length_in(@$_), @arrays;
my @row_major = map {my $i = $_; [map $$_[$i], @arrays]} 0 .. $#{$arrays[0]};
my $format    = join '  ', map "%-${_}s", @lengths;

join "\n", map strip(sprintf($format, @$_)), @row_major;
__
meta::internal_function('temporary_name', <<'__');
use File::Temp 'tempfile';
my (undef, $temporary_filename) = tempfile("$0." . 'X' x 4, OPEN => 0);
$temporary_filename;
__
meta::internal_function('translate_backtrace', <<'__');
my ($trace) = @_;
$trace =~ s/\(eval (\d+)\)/$locations{$1 - 1}/g;
$trace;
__
meta::internal_function('with_cwd', <<'__');
my ($dir, $f) = @_;
my $cwd = cwd();

my @result = eval {chdir $dir && &$f()};
chdir $cwd;
die $@ if $@;
wantarray ? @result : $result[0];
__
meta::internal_function('with_exported', <<'__');
# Like exported(), but removes the file after running some function.
# Usage is with_exported(@files, sub {...});
my $f      = pop @_;
my $name   = exported(@_);
my $result = eval {&$f($name)};
terminal::warning("$@ when running with_exported()") if $@;
file::nuke($name);
$result;
__
meta::internal_function('with_fork', <<'__');
my (@args) = @_;
my $f = pop @args;

return process->new($child_pid) if my $child_pid = fork;

# This is the child process. Disable saving to prevent contention, and then
# exit with the given status code.
*::save = sub {};
exit &$f(@args);
__
meta::library('process', <<'__');
package process;

sub new  {my ($class, $pid) = @_; bless \$pid, $class}
sub kill {my ($self, $signal) = @_; ::kill $signal // 'KILL', $$self; $self->wait()}
sub term {my ($self) = @_; $self->kill('TERM')}
sub int  {my ($self) = @_; $self->kill('INT')}
sub stop {my ($self) = @_; $self->kill('STOP')}
sub cont {my ($self) = @_; $self->kill('CONT')}

sub wait {my ($self) = @_; ::wait($$self)}
__
meta::library('shell', <<'__');
# Functions for shell parsing and execution.
package shell;
use Term::ReadLine;

sub tokenize {grep length, split /\s+|("[^"\\]*(?:\\.)?")/o, join ' ', @_};

sub parse {
  my ($fn, @args) = @_;
  s/^"(.*)"$/\1/o, s/\\\\"/"/go for @args;
  {function => $fn, args => [@args]}}

sub execute {
  my %command = %{$_[0]};
  if (exists $externalized_functions{$command{function}})
    {&{"::$command{function}"}(@{$command{args}})}
  elsif (exists $externalized_functions{'method-missing'})
    {::method_missing($command{function}, @{$command{args}})}
  else
    {die "undefined command: $command{function}"}}

sub run {execute(parse(tokenize(@_)))}

sub prompt {
  my %options = @_;
  my $name    = $options{name} // ::name();

  my $indicators = join '', map &{"::$_"}(), ::select_keys('--namespace' => 'indicator');
  my $prefix     = $transient{repl_prefix} // '';

  "$prefix\033[1;32m$name\033[0;0m$indicators "}

sub repl {
  my %options = @_;

  my $term = new Term::ReadLine "$0 shell";
  $term->ornaments(0);
  my $attribs = $term->Attribs;
  $attribs->{completion_entry_function} = $attribs->{list_completion_function};

  my $autocomplete = $options{autocomplete} || sub {[sort(keys %data), grep !/-/, sort keys %externalized_functions]};
  my $prompt       = $options{prompt}       || \&prompt;
  my $parse        = $options{parse}        || sub {parse(tokenize(@_))};
  my $output       = $options{output}       || sub {print join("\n", @_), "\n" if grep length, @_};
  my $command      = $options{command}      || sub {my ($command) = @_; ::around_hook('shell-command', $command, sub {&$output(::dangerous('', sub {execute($command)}))})};

  length $_ && &$command(&$parse($_)) while ($attribs->{completion_word} = &$autocomplete(), defined($_ = $term->readline(&$prompt())))}
__
meta::library('terminal', <<'__');
# Functions for nice-looking terminal output.
package terminal;

my $process = ::name();

sub message {print STDERR "[$_[0]] $_[1]\n"}
sub color {
  my ($name, $color) = @_;
  *{"terminal::$name"} = sub {chomp($_), print STDERR "\033[1;30m$process(\033[1;${color}m$name\033[1;30m)\033[0;0m $_\n" for map join('', $_), @_}}

my %preloaded = (info => 32, progress => 32, state => 34, debug => 34, warning => 33, error => 31);
color $_, $preloaded{$_} for keys %preloaded;
__
meta::message_color('cc', '36');
meta::message_color('state', 'purple');
meta::message_color('states', 'yellow');
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
function::expanded-bootstrap            ngkWm7nFPxTICYZLKkdsabU4dFOpRBuvlpXdJsOvUGU
function::export                        a/OWMZFsFsvHwnlvCBEZMg8WhZ9IP9ZwqJZBj7Garq8
function::extern                        EG6Lhkqo4xNy3xxWJgDv22Ij987xFAYgtwSXWNzwgDw
function::grep                          V4XnzOTDllwCW/e2EMuFITuX/q6stdhIQaCCOPo6ob4
function::hash                          +KkJP3kfAnYIXy1BBSEDdfzhnLpQPqz52RJpwgk+OCc
function::hook                          rhEyOmCy+ZDyO727aPXfTrEtkOOGaGKcezlkYf9BK0k
function::hooks                         2rZQUo7p9iu9gmM4wR+G6/RukNXgAk+ZcFepCgnfef8
function::identity                      6+JeD3ohDjevZF5UG7V3iRFkxIwp4KwwibpVc9dwcm4
function::import                        4TKj8ayYsR8EtPFO+Jv3FK2Rv4j4uTAOC77twB9GBkE
function::initial-state                 E45KKMXa7YYzleuQNeMa+2XTkvSxKw40KaWN0lHLhV0
function::is                            R4wqjFaGUQnk3q8Ru9HEoyuoiTd0nEXs8655T50bO3I
function::journal                       x4zLMyhSRotanzZQXb2H2hdCVADixqREcsFJjehPv5I
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
function::save                          k2yC8b9dcIeXDeKmNQbxMY8j7svl4kyLoG+XAJY/E/8
function::save-state                    KyoEB2NXy0ko1IYUVujXqEfbNbsoDckiKjwn8+zjvxs
function::serialize                     C0ZKGKNZKobNO0kmIPR2x0gXt6ic2YkCBCrCWqApodQ
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
meta::retriever('file', '-f $_[0] ? file::read($_[0]) : undef;');
meta::retriever('global', <<'__');
# Returns the global data stashed at the end of this perl object
$_[0] eq 'self' ? $global_data : undef;
__
meta::retriever('id', '$_[0] =~ /^id::/ ? substr($_[0], 4) : undef;');
meta::retriever('object', <<'__');
# Fetch a property from another Perl object. This uses the 'cat' function.
return undef unless $_[0] =~ /^object::(.*?)::(.*)$/ && -x $1 && qx|$1 is '$2'|;
join '', qx|$1 cat '$2'|;
__
meta::retriever('perl', <<'__');
# Lets you use the result of evaluating some Perl expression
return undef unless $_[0] =~ /^perl::(.*)$/;
eval $1;
__
internal::main();

__DATA__
