/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1999 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

/******

  This file contains the list of all parsed CSS keywords
  See nsCSSKeywords.h for access to the enum values for keywords

  It is designed to be used as inline input to nsCSSKeywords.cpp *only*
  through the magic of C preprocessing.

  All entires must be enclosed in the macro CSS_KEY which will have cruel
  and unusual things done to it

  It is recommended (but not strictly necessary) to keep all entries
  in alphabetical order

  Underbars in keywords are automagically converted into hyphens

 ******/


CSS_KEY(_moz_all)
CSS_KEY(_moz_bg_inset)
CSS_KEY(_moz_bg_outset)
CSS_KEY(_moz_center)
CSS_KEY(_moz_default)
CSS_KEY(_moz_right)
CSS_KEY(_moz_pre_wrap)
CSS_KEY(_moz_scrollbars_none)
CSS_KEY(_moz_scrollbars_horizontal)
CSS_KEY(_moz_scrollbars_vertical)
CSS_KEY(above)
CSS_KEY(absolute)
CSS_KEY(activeborder)
CSS_KEY(activecaption)
CSS_KEY(alias)
CSS_KEY(all)
CSS_KEY(always)
CSS_KEY(appworkspace)
CSS_KEY(arabic_indic)
CSS_KEY(armenian)
CSS_KEY(auto)
CSS_KEY(avoid)
CSS_KEY(background)
CSS_KEY(baseline)
CSS_KEY(behind)
CSS_KEY(below)
CSS_KEY(bengali)
CSS_KEY(bidi_override)
CSS_KEY(blink)
CSS_KEY(block)
CSS_KEY(bold)
CSS_KEY(bolder)
CSS_KEY(border_box)
CSS_KEY(both)
CSS_KEY(bottom)
CSS_KEY(button)
CSS_KEY(buttonface)
CSS_KEY(buttonhighlight)
CSS_KEY(buttonshadow)
CSS_KEY(buttontext)
CSS_KEY(capitalize)
CSS_KEY(caption)
CSS_KEY(captiontext)
CSS_KEY(cell)
CSS_KEY(center)
CSS_KEY(center_left)
CSS_KEY(center_right)
CSS_KEY(ch)
CSS_KEY(circle)
CSS_KEY(cjk_earthly_branch)
CSS_KEY(cjk_heavenly_stem)
CSS_KEY(cjk_ideographic)
CSS_KEY(close_quote)
CSS_KEY(cm)
CSS_KEY(code)
CSS_KEY(collapse)
CSS_KEY(compact)
CSS_KEY(condensed)
CSS_KEY(content_box)
CSS_KEY(context_menu)
CSS_KEY(continuous)
CSS_KEY(copy)
CSS_KEY(count_down)
CSS_KEY(count_up)
CSS_KEY(count_up_down)
CSS_KEY(crop)
CSS_KEY(cross)
CSS_KEY(crosshair)
CSS_KEY(dashed)
CSS_KEY(decimal)
CSS_KEY(decimal_leading_zero)
CSS_KEY(default)
CSS_KEY(deg)
CSS_KEY(desktop)
CSS_KEY(devanagari)
CSS_KEY(dialog)
CSS_KEY(digits)
CSS_KEY(disabled)
CSS_KEY(disc)
CSS_KEY(document)
CSS_KEY(dotted)
CSS_KEY(double)
CSS_KEY(e_resize)
CSS_KEY(element)
CSS_KEY(elements)
CSS_KEY(em)
CSS_KEY(embed)
CSS_KEY(enabled)
CSS_KEY(ex)
CSS_KEY(expanded)
CSS_KEY(extra_condensed)
CSS_KEY(extra_expanded)
CSS_KEY(far_left)
CSS_KEY(far_right)
CSS_KEY(fast)
CSS_KEY(faster)
CSS_KEY(field)
CSS_KEY(fixed)
CSS_KEY(georgian)
CSS_KEY(grab)
CSS_KEY(grabbing)
CSS_KEY(grad)
CSS_KEY(graytext)
CSS_KEY(groove)
CSS_KEY(gujarati)
CSS_KEY(gurmukhi)
CSS_KEY(hebrew)
CSS_KEY(help)
CSS_KEY(hidden)
CSS_KEY(hide)
CSS_KEY(high)
CSS_KEY(higher)
CSS_KEY(highlight)
CSS_KEY(highlighttext)
CSS_KEY(hiragana)
CSS_KEY(hiragana_iroha)
CSS_KEY(horizontal)
CSS_KEY(hz)
CSS_KEY(icon)
CSS_KEY(ignore)
CSS_KEY(in)
CSS_KEY(inactiveborder)
CSS_KEY(inactivecaption)
CSS_KEY(inactivecaptiontext)
CSS_KEY(info)
CSS_KEY(infobackground)
CSS_KEY(infotext)
CSS_KEY(inherit)
CSS_KEY(inline)
CSS_KEY(inline_block)
CSS_KEY(inline_table)
CSS_KEY(inset)
CSS_KEY(inside)
CSS_KEY(invert)
CSS_KEY(italic)
CSS_KEY(japanese_formal)
CSS_KEY(japanese_informal)
CSS_KEY(justify)
CSS_KEY(kannada)
CSS_KEY(katakana)
CSS_KEY(katakana_iroha)
CSS_KEY(khmer)
CSS_KEY(khz)
CSS_KEY(landscape)
CSS_KEY(lao)
CSS_KEY(large)
CSS_KEY(larger)
CSS_KEY(left)
CSS_KEY(left_side)
CSS_KEY(leftwards)
CSS_KEY(level)
CSS_KEY(lighter)
CSS_KEY(line_through)
CSS_KEY(list)
CSS_KEY(list_item)
CSS_KEY(loud)
CSS_KEY(low)
CSS_KEY(lower)
CSS_KEY(lower_alpha)
CSS_KEY(lower_greek)
CSS_KEY(lower_latin)
CSS_KEY(lower_roman)
CSS_KEY(lowercase)
CSS_KEY(ltr)
CSS_KEY(malayalam)
CSS_KEY(margin_box)
CSS_KEY(marker)
CSS_KEY(medium)
CSS_KEY(menu)
CSS_KEY(menutext)
CSS_KEY(message_box)
CSS_KEY(middle)
CSS_KEY(mix)
CSS_KEY(mm)
CSS_KEY(move)
CSS_KEY(ms)
CSS_KEY(myanmar)
CSS_KEY(n_resize)
CSS_KEY(narrower)
CSS_KEY(ne_resize)
CSS_KEY(no_close_quote)
CSS_KEY(no_open_quote)
CSS_KEY(no_repeat)
CSS_KEY(none)
CSS_KEY(normal)
CSS_KEY(noshade)
CSS_KEY(nowrap)
CSS_KEY(nw_resize)
CSS_KEY(oblique)
CSS_KEY(once)
CSS_KEY(open_quote)
CSS_KEY(oriya)
CSS_KEY(outset)
CSS_KEY(outside)
CSS_KEY(overline)
CSS_KEY(padding_box)
CSS_KEY(paragraph)
CSS_KEY(pc)
CSS_KEY(persian)
CSS_KEY(pointer)
CSS_KEY(portrait)
CSS_KEY(pre)
CSS_KEY(pt)
CSS_KEY(pull_down_menu)
CSS_KEY(px)
CSS_KEY(rad)
CSS_KEY(read_only)
CSS_KEY(read_write)
CSS_KEY(relative)
CSS_KEY(repeat)
CSS_KEY(repeat_x)
CSS_KEY(repeat_y)
CSS_KEY(ridge)
CSS_KEY(right)
CSS_KEY(right_side)
CSS_KEY(rightwards)
CSS_KEY(rtl)
CSS_KEY(run_in)
CSS_KEY(s)
CSS_KEY(s_resize)
CSS_KEY(scroll)
CSS_KEY(scrollbar)
CSS_KEY(se_resize)
CSS_KEY(select_all)
CSS_KEY(select_before)
CSS_KEY(select_after)
CSS_KEY(select_same)
CSS_KEY(select_menu)
CSS_KEY(semi_condensed)
CSS_KEY(semi_expanded)
CSS_KEY(separate)
CSS_KEY(show)
CSS_KEY(silent)
CSS_KEY(simp_chinese_formal)
CSS_KEY(simp_chinese_informal)
CSS_KEY(slow)
CSS_KEY(slower)
CSS_KEY(small)
CSS_KEY(small_caps)
CSS_KEY(small_caption)
CSS_KEY(smaller)
CSS_KEY(soft)
CSS_KEY(solid)
CSS_KEY(spell_out)
CSS_KEY(spinning)
CSS_KEY(square)
CSS_KEY(static)
CSS_KEY(status_bar)
CSS_KEY(sub)
CSS_KEY(super)
CSS_KEY(sw_resize)
CSS_KEY(table)
CSS_KEY(table_caption)
CSS_KEY(table_cell)
CSS_KEY(table_column)
CSS_KEY(table_column_group)
CSS_KEY(table_footer_group)
CSS_KEY(table_header_group)
CSS_KEY(table_row)
CSS_KEY(table_row_group)
CSS_KEY(tamil)
CSS_KEY(telugu)
CSS_KEY(text)
CSS_KEY(text_bottom)
CSS_KEY(text_top)
CSS_KEY(thai)
CSS_KEY(thick)
CSS_KEY(thin)
CSS_KEY(threeddarkshadow)
CSS_KEY(threedface)
CSS_KEY(threedhighlight)
CSS_KEY(threedlightshadow)
CSS_KEY(threedshadow)
CSS_KEY(toggle)
CSS_KEY(top)
CSS_KEY(trad_chinese_formal)
CSS_KEY(trad_chinese_informal)
CSS_KEY(transparent)
CSS_KEY(tri_state)
CSS_KEY(ultra_condensed)
CSS_KEY(ultra_expanded)
CSS_KEY(underline)
CSS_KEY(upper_alpha)
CSS_KEY(upper_latin)
CSS_KEY(upper_roman)
CSS_KEY(uppercase)
CSS_KEY(urdu)
CSS_KEY(vertical)
CSS_KEY(visible)
CSS_KEY(w_resize)
CSS_KEY(wait)
CSS_KEY(wider)
CSS_KEY(window)
CSS_KEY(windowframe)
CSS_KEY(windowtext)
CSS_KEY(workspace)
CSS_KEY(write_only)
CSS_KEY(x_fast)
CSS_KEY(x_high)
CSS_KEY(x_large)
CSS_KEY(x_loud)
CSS_KEY(x_low)
CSS_KEY(x_slow)
CSS_KEY(x_small)
CSS_KEY(x_soft)
CSS_KEY(xx_large)
CSS_KEY(xx_small)
