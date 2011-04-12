// Self-modifying Perl web interface | Spencer Tipping
// Licensed under the terms of the MIT source code license

// Introduction.
// This interface lets users open self-modifying Perl files as web pages. Attributes will be shown in a list, and each one can be expanded to show its value.

caterwaul.deglobalize().clone('std seq continuation montenegro parser')(function ($) {
  jQuery = fn_[$.apply(this, arguments)] /se[_.prototype = $.prototype],

  $(fn_[$('body').empty().append(the_interface),
        where*[original_html = $('body').html(),

               parse_attributes(s) = l*[unescape(s)           = s.replace(/\\(['\\])/g, '$1'),

                                        namespace             = peg[c(/meta::(\w+)/, 7) >> fn[xs][xs[1]]],
                                        string                = peg[c('\'') % (c(/[^\\']/, 1) / (c(/\\(.)/, 2) >> unescape))[0] % c('\'') >> fn[xs][xs[1]]],
                                        here_document         = peg[c('<<') % string >> fn[xs][xs[1]]],

                                        invocation_common     = peg[namespace % c('(') % string % c(/,\s*/, 1) >> fn[xs][{namespace: xs[0], attribute: xs[2]}]],

                                        short_form_invocation = peg[invocation_common % string % c(');') % c('\n') >> fn[xs][xs[0] /se[_.value = xs[1]]]],

                                        long_form_invocation  = l*[beginning            = peg[invocation_common % here_document % c(');\n') >> fn[xs][xs[0] /se[_.marker = xs[1]]]],
                                                                   ending(end)          = peg[(-c(end) % c(fn_[1]))[0] % c(end) >> fn[xs][seq[~xs[0] *[_[1]]].join('')]],
                                                                   parse_heredoc(input) = l[beginning_result = beginning(input) || {result: {marker: fn_[false]}}] in
                                                                                          ending(beginning_result.result.marker)(beginning_result)] in
                                                                parse_heredoc,

                                        attribute             = peg[short_form_invocation / long_form_invocation],
                                        attributes            = peg[attribute[0]],

                                        trim_bootstrap(s)     = s.replace(/^(?:(?!meta::).)*/, '')] in attributes(trim_bootstrap(s)),

               the_interface = html[pre(document.createTextNode(JSON.stringify(parse_attributes(original_html))))]]])})($.noConflict(true));

// Generated by SDoc 
