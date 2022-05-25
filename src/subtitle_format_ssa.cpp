// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "subtitle_format_ssa.h"

#include "ass_attachment.h"
#include "ass_dialogue.h"
#include "ass_info.h"
#include "ass_file.h"
#include "ass_style.h"
#include "text_file_writer.h"
#include "version.h"

#include <libaegisub/format.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace {
std::string replace_commas(std::string str) {
	boost::replace_all(str, ",", ";");
	return str;
}

std::string strip_newlines(std::string str) {
	boost::replace_all(str, "\n", "");
	boost::replace_all(str, "\r", "");
	return str;
}
}

void SsaSubtitleFormat::WriteFile(const AssFile *src, agi::fs::path const& filename, agi::vfr::Framerate const&, std::string const& encoding) const {
	TextFileWriter file(filename, encoding);

	file.WriteLineToFile("[Script Info]");
	file.WriteLineToFile(std::string("; Script generated by Aegisub ") + GetAegisubLongVersionString());
	file.WriteLineToFile("; http://www.aegisub.org/");
	for (auto const& line : src->Info)
		file.WriteLineToFile(boost::iequals(line.Key(), "scripttype") ? "ScriptType: v4.00" : line.GetEntryData());

	file.WriteLineToFile("");
	file.WriteLineToFile("[V4 Styles]");
	file.WriteLineToFile("Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, TertiaryColour, BackColour, Bold, Italic, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, AlphaLevel, Encoding");
	for (auto const& line : src->Styles)
		file.WriteLineToFile(agi::format("Style: %s,%s,%g,%s,%s,0,%s,%d,%d,%d,%g,%g,%d,%d,%d,%d,0,%i"
			, line.name, line.font, line.fontsize
			, line.primary.GetSsaFormatted()
			, line.secondary.GetSsaFormatted()
			, line.shadow.GetSsaFormatted()
			, (line.bold? -1 : 0), (line.italic ? -1 : 0)
			, line.borderstyle, line.outline_w, line.shadow_w, AssStyle::AssToSsa(line.alignment)
			, line.Margin[0], line.Margin[1], line.Margin[2], line.encoding));

	file.WriteLineToFile("");
	file.WriteLineToFile("[Fonts]");
	for (auto const& line : src->Attachments) {
		if (line.Group() == AssEntryGroup::FONT)
			file.WriteLineToFile(line.GetEntryData());
	}

	file.WriteLineToFile("");
	file.WriteLineToFile("[Graphics]");
	for (auto const& line : src->Attachments) {
		if (line.Group() == AssEntryGroup::GRAPHIC)
			file.WriteLineToFile(line.GetEntryData());
	}

	file.WriteLineToFile("");
	file.WriteLineToFile("[Events]");
	file.WriteLineToFile("Format: Marked, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text");
	for (auto const& line : src->Events)
		file.WriteLineToFile(agi::format("%s: Marked=0,%s,%s,%s,%s,%d,%d,%d,%s,%s"
			, (line.Comment ? "Comment" : "Dialogue")
			, line.Start.GetAssFormatted(), line.End.GetAssFormatted()
			, replace_commas(line.Style), replace_commas(line.Actor)
			, line.Margin[0], line.Margin[1], line.Margin[2]
			, replace_commas(line.Effect)
			, strip_newlines(line.Text)));
}
