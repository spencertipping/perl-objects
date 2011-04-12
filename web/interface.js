// Self-modifying Perl web interface | Spencer Tipping
// Licensed under the terms of the MIT source code license

// Introduction.
// This interface lets users open self-modifying Perl files as web pages. Attributes will be shown in a list, and each one can be expanded to show its value.

typeof loaded === 'undefined' &&
caterwaul.clone('std seq continuation montenegro parser')(function ($) {
  loaded = true;
  $(fn_[$('head').append($('link')),
        $('body').empty().append(loading_interface),
        setTimeout(_, 100) /cps[$('body').empty().append(the_interface())],

  where*[original_html           = $('body').html(),

         parse_attributes(s)     = l*[namespace             = peg[[c(/[\s\n]+/, 1)] % c(/meta::(\w+)/, 7) >> fn[xs][xs[1][1]]],
                                      string                = peg[c('\'') % (c(/[^\\']/, 1) / (c(/\\(.)/, 2) >> fn[xs][xs[1]]))[0] % c('\'') >> fn[xs][xs[1].join('')]],
                                      here_document         = peg[c('<<') % string >> fn[xs][xs[1]]],

                                      invocation_common     = peg[namespace % c('(') % string % c(/,\s*/, 1) >> fn[xs][{namespace: xs[0], attribute: xs[2]}]],

                                      short_form_invocation = peg[invocation_common % string % c(');\n') >> fn[xs][xs[0] /se[_.value = xs[1]]]],

                                      long_form_invocation  = l*[beginning            = peg[invocation_common % here_document % c(');\n') >> fn[xs][xs[0] /se[_.marker = xs[1]]]],
                                                                 ending(end)          = peg[(reject(c(end)) % c(fn_[1]))[0] % c(end) >> fn[xs][seq[~xs[0] *[_[1]]].join('')]],
                                                                 parse_heredoc(input) = l*[begin = beginning(input),
                                                                                           end   = begin.result && ending(begin.result.marker)(begin)] in
                                                                                        end /se[_.result = begin.result /se[_.value = end.result], when[begin.result]]] in
                                                              parse_heredoc,

                                      attribute             = peg[short_form_invocation / long_form_invocation],
                                      attributes            = peg[attribute[0]],

                                      unescape(s)           = s.replace(/&lt;/gi, '<').replace(/&gt;/gi, '>').replace(/&amp;/gi, '&'),
                                      trim_bootstrap(s)     = s.replace(/^(?:(?!\nmeta::\w+\()(?:.|\n))*\n/, '')] in

                                   attributes(trim_bootstrap(unescape(s))),

         partition(name, xs)     = html[div.partition(h1.namespace(name), div.attributes)] /se[_.find('.attributes').append(seq[~xs *+attribute])],
         attribute(x)            = html[div.attribute(a.name(x.attribute), pre.contents(x.value) /css({display: 'none'}))] /se[
                                   _.find('.name').clickable(fn_[_.children('.contents').toggle()])],

         separate_namespaces(xs) = l[namespace_object = seq[~xs *[[_.namespace, true]]].object()] in seq[sk[namespace_object] *[[_, seq[~xs %x[x.namespace === _]]]]].object(),
         namespace_sections(xs)  = l[partitions = separate_namespaces(xs)] in seq[sp[partitions] *[partition(_[0], _[1])]],

         loading_interface       = html[h1('parsing object state...')],

         the_interface()         = l*[parsed                  = parse_attributes(original_html),
                                      model                   = l[partitioned = separate_namespaces(parsed)] in seq[sp[partitioned] *~[[_[0], !(_[1] *[[_.attribute, _.value]])]]].object(),

                                      _ = console.log(model),

                                      instructions            = html[h1('Usage instructions'),
                                                                     p('This HTML file is also a valid Perl script that can rewrite itself to update its state. (See "',
                                                                       a('writing self-modifying Perl') *href('http://github.com/spencertipping/writing-self-modifying-perl') *target('_blank'),
                                                                       '" for an explanation of how it works.) ',
                                                                       'This web interface shows you which attributes have been defined. You can modify the attributes by running the script ' +
                                                                       'on the command line.'),
                                                                     ol(li('Set the executable bit of the script: ', code('chmod u+x script-name.html')),
                                                                        li('Open a REPL session: ', code('./script-name.html shell')),
                                                                        li('If unsure, use the ', code('lock'), ' REPL command to prevent any unwanted modification.')),
                                                                     p('You should probably be familiar with how these scripts work before running the REPL.')],

                                      metadata_section        = l[identity = model.data['permanent-identity'] || '[unidentified]', author = model.data.author || '[no author specified]',
                                                                  license  = model.data.license || '[no license specified]'] in
                                                                html[div.metadata(h1.identity(identity), h1.author(author), pre.license(license), div.instructions(instructions))],

                                      attribute_section       = html[div.attributes(namespace_sections(parse_attributes(original_html)))]] in

                                   [metadata_section, attribute_section]]])})($);

// Generated by SDoc 
