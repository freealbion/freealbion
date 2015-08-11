/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2015 Florian Ziesche
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "albion_script.hpp"
#include <floor/lang/source_types.hpp>
#include <floor/lang/grammar.hpp>
#include <floor/lang/lang_context.hpp>
#include <floor/constexpr/const_string.hpp>

class albion_script_lexer final : public lexer {
public:
	static void lex(translation_unit& tu);
	
protected:
	// NOTE: these are all the functions to lex any token
	// NOTE: every lex_* function has to return an iterator to the character following the lexed token (or .end())
	// and must also set the iter parameter to this iterator position!
	
	static lex_return_type lex_keyword_or_identifier(const translation_unit& tu,
													 source_iterator& iter,
													 const source_iterator& source_end);
	static lex_return_type lex_decimal_constant(const translation_unit& tu,
												source_iterator& iter,
												const source_iterator& source_end);
	static lex_return_type lex_comment(const translation_unit& tu,
									   source_iterator& iter,
									   const source_iterator& source_end);
	
	// static class
	albion_script_lexer(const albion_script_lexer&) = delete;
	~albion_script_lexer() = delete;
	albion_script_lexer& operator=(const albion_script_lexer&) = delete;
	
};

void albion_script_lexer::lex(translation_unit& tu) {
	// tokens reserve strategy: "4 chars : 1 token" seems like a good ratio for now
	tu.tokens.reserve(tu.source.size() / 4);
	
	// lex
	for(auto src_begin = tu.source.data(), src_end = tu.source.data() + tu.source.size(), char_iter = src_begin;
		char_iter != src_end;
		/* NOTE: char_iter is incremented in the individual lex_* functions or whitespace case: */) {
		switch(*char_iter) {
			// keyword or identifier
			case '_':
			case 'a': case 'b': case 'c': case 'd':
			case 'e': case 'f': case 'g': case 'h':
			case 'i': case 'j': case 'k': case 'l':
			case 'm': case 'n': case 'o': case 'p':
			case 'q': case 'r': case 's': case 't':
			case 'u': case 'v': case 'w': case 'x':
			case 'y': case 'z':
			case 'A': case 'B': case 'C': case 'D':
			case 'E': case 'F': case 'G': case 'H':
			case 'I': case 'J': case 'K': case 'L':
			case 'M': case 'N': case 'O': case 'P':
			case 'Q': case 'R': case 'S': case 'T':
			case 'U': case 'V': case 'W': case 'X':
			case 'Y': case 'Z': {
				source_range range { char_iter, char_iter };
				const auto ret = lex_keyword_or_identifier(tu, char_iter, src_end);
				if(!ret.first) break;
				range.end = ret.second;
				tu.tokens.emplace_back(SOURCE_TOKEN_TYPE::IDENTIFIER, range);
				break;
			}
				
			// decimal constant
			case '-': case '0': case '1': case '2':
			case '3': case '4': case '5': case '6':
			case '7': case '8': case '9': {
				source_range range { char_iter, char_iter };
				const auto ret = lex_decimal_constant(tu, char_iter, src_end);
				if(!ret.first) break;
				range.end = ret.second;
				tu.tokens.emplace_back(SOURCE_TOKEN_TYPE::INTEGER_CONSTANT, range);
				break;
			}
				
			// ';' -> comment
			case ';': {
				// comment
				lex_comment(tu, char_iter, src_end);
				break;
			}
				
			// whitespace
			// "space, horizontal tab, new-line, vertical tab, and form-feed"
			case ' ': case '\t': case '\n': case '\v':
			case '\f':
				// continue
				++char_iter;
				break;
				
			// invalid char
			default: {
				const string invalid_char = (is_printable_char(char_iter) ? string(1, *char_iter) : "<unprintable>");
				handle_error(tu, char_iter, "invalid character \'" + invalid_char + "\' (" +
							 to_string(0xFFu & (uint32_t)*char_iter) + ")");
				break;
			}
		}
	}
}

lexer::lex_return_type albion_script_lexer::lex_keyword_or_identifier(const translation_unit& tu floor_unused,
																	  source_iterator& iter,
																	  const source_iterator& source_end) {
	for(++iter; iter != source_end; ++iter) {
		switch(*iter) {
			// valid keyword and identifier characters
			case '_':
			case 'a': case 'b': case 'c': case 'd':
			case 'e': case 'f': case 'g': case 'h':
			case 'i': case 'j': case 'k': case 'l':
			case 'm': case 'n': case 'o': case 'p':
			case 'q': case 'r': case 's': case 't':
			case 'u': case 'v': case 'w': case 'x':
			case 'y': case 'z':
			case 'A': case 'B': case 'C': case 'D':
			case 'E': case 'F': case 'G': case 'H':
			case 'I': case 'J': case 'K': case 'L':
			case 'M': case 'N': case 'O': case 'P':
			case 'Q': case 'R': case 'S': case 'T':
			case 'U': case 'V': case 'W': case 'X':
			case 'Y': case 'Z':
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
			case '8': case '9':
				continue;
				
			// anything else -> done, return end iter
			default:
				return { true, iter };
		}
	}
	return { true, iter }; // eof
}

lexer::lex_return_type albion_script_lexer::lex_decimal_constant(const translation_unit& tu floor_unused,
																 source_iterator& iter,
																 const source_iterator& source_end) {
	for(++iter; iter != source_end; ++iter) {
		switch(*iter) {
			// valid decimal constant characters
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
			case '8': case '9':
				continue;
				
			// anything else -> done, return end iter
			default:
				return { true, iter };
		}
	}
	return { true, iter }; // eof
}

lexer::lex_return_type albion_script_lexer::lex_comment(const translation_unit& tu floor_unused,
														source_iterator& iter,
														const source_iterator& source_end) {
	// NOTE: we already made sure that this must be a comment and the next character is a '/' or '*'
	++iter;
	
	// single-line comment
	for(++iter; iter != source_end; ++iter) {
		// newline signals end of single-line comment
		if(*iter == '\n') {
			return { true, iter };
		}
	}
	return { true, iter }; // eof is okay in a single-line comment
}

static struct albion_parser_grammar {
	// all grammar rule objects
#if !defined(FLOOR_DEBUG_PARSER) && !defined(FLOOR_DEBUG_PARSER_SET_NAMES)
#define FLOOR_GRAMMAR_OBJECTS(...) grammar_rule __VA_ARGS__;
#else
#define FLOOR_GRAMMAR_OBJECTS(...) grammar_rule __VA_ARGS__; \
	/* in debug mode, also set the debug name of each grammar rule object */ \
	void set_debug_names() { \
		string names { #__VA_ARGS__ }; \
		set_debug_name(names, __VA_ARGS__); \
	} \
	void set_debug_name(string& names, grammar_rule& obj) { \
		const auto comma_pos = names.find(","); \
		obj.debug_name = names.substr(0, comma_pos); \
		names.erase(0, comma_pos + 1 + (comma_pos+1 < names.size() && names[comma_pos+1] == ' ' ? 1 : 0)); \
	} \
	template <typename... grammar_objects> void set_debug_name(string& names, grammar_rule& obj, grammar_objects&... objects) { \
		set_debug_name(names, obj); \
		set_debug_name(names, objects...); \
	}
#endif
	
	FLOOR_GRAMMAR_OBJECTS(translation_unit, statement,
						  ambient,
						  camera_jump, camera_lock, camera_move, camera_unlock,
						  clear_quest_bit, do_event_chain,
						  fade_from_black, fade_from_white, fade_to_black, fade_to_white,
						  fill_screen, fill_screen_0,
						  load_pal,
						  npc_jump, npc_lock, npc_move, npc_off,
						  npc_on, npc_text, npc_turn, npc_unlock,
						  party_jump, party_member_text, party_move, party_off,
						  party_on, party_turn,
						  pause, play, play_anim,
						  show_map, show_pic, show_picture,
						  song, sound, sound_effect, sound_fx_off,
						  start_anim, stop_anim,
						  text, update)
	
	struct keyword_matcher :
	public parser_node_base<keyword_matcher> {
		const char* keyword;
		const size_t keyword_len;
		
		template <size_t len> constexpr keyword_matcher(const char (&keyword_)[len]) noexcept :
		keyword(keyword_), keyword_len(len - 1 /* -1, b/c of \0 */) {}
		
		match_return_type match(parser_context& ctx) const {
#if defined(FLOOR_DEBUG_PARSER) && !defined(FLOOR_DEBUG_PARSER_MATCHES_ONLY)
			ctx.print_at_depth("matching KEYWORD");
#endif
			if(!ctx.at_end() &&
			   ctx.iter->first == SOURCE_TOKEN_TYPE::IDENTIFIER &&
			   ctx.iter->second.equal(keyword, keyword_len)) {
				match_return_type ret { ctx.iter };
				ctx.next();
				return ret;
			}
			return { false, false, {} };
		}
	};
	
	albion_parser_grammar() {
		// fixed token type matchers:
		static constexpr literal_matcher<const char*, SOURCE_TOKEN_TYPE::INTEGER_CONSTANT> INT_CONSTANT {};
		
		//
		static constexpr keyword_matcher AMBIENT("ambient"),
		CAMERA_JUMP("camera_jump"), CAMERA_LOCK("camera_lock"), CAMERA_MOVE("camera_move"), CAMERA_UNLOCK("camera_unlock"),
		CLEAR_QUEST_BIT("clear_quest_bit"), DO_EVENT_CHAIN("do_event_chain"),
		FADE_FROM_BLACK("fade_from_black"), FADE_FROM_WHITE("fade_from_white"), FADE_TO_BLACK("fade_to_black"), FADE_TO_WHITE("fade_to_white"),
		FILL_SCREEN("fill_screen"), FILL_SCREEN_0("fill_screen_0"),
		LOAD_PAL("load_pal"),
		NPC_JUMP("npc_jump"), NPC_LOCK("npc_lock"), NPC_MOVE("npc_move"), NPC_OFF("npc_off"),
		NPC_ON("npc_on"), NPC_TEXT("npc_text"), NPC_TURN("npc_turn"), NPC_UNLOCK("npc_unlock"),
		PARTY_JUMP("party_jump"), PARTY_MEMBER_TEXT("party_member_text"), PARTY_MOVE("party_move"), PARTY_OFF("party_off"),
		PARTY_ON("party_on"), PARTY_TURN("party_turn"),
		PAUSE("pause"), PLAY("play"), PLAY_ANIM("play_anim"),
		SHOW_MAP("show_map"), SHOW_PIC("show_pic"), SHOW_PICTURE("show_picture"),
		SONG("song"), SOUND("sound"), SOUND_EFFECT("sound_effect"), SOUND_FX_OFF("sound_fx_off"),
		START_ANIM("start_anim"), STOP_ANIM("stop_anim"),
		TEXT("text"), UPDATE("update");
		
#if defined(FLOOR_DEBUG_PARSER) || defined(FLOOR_DEBUG_PARSER_SET_NAMES)
		set_debug_names();
#endif
		
		// grammar:
		translation_unit = *statement;
		statement = (ambient | camera_jump | camera_lock | camera_move |
					 camera_unlock | clear_quest_bit | do_event_chain |
					 fade_from_black | fade_from_white | fade_to_black |
					 fade_to_white | fill_screen | fill_screen_0 | load_pal |
					 npc_jump | npc_lock | npc_move | npc_off | npc_on |
					 npc_text | npc_turn | npc_unlock | party_jump |
					 party_member_text | party_move | party_off | party_on |
					 party_turn | pause | play | play_anim | show_map |
					 show_pic | show_picture | song | sound | sound_effect |
					 sound_fx_off | start_anim | stop_anim | text | update);
		ambient = AMBIENT & INT_CONSTANT;
		camera_jump = CAMERA_JUMP & INT_CONSTANT & INT_CONSTANT;
		camera_lock = CAMERA_LOCK;
		camera_move = CAMERA_MOVE & INT_CONSTANT & INT_CONSTANT;
		camera_unlock = CAMERA_UNLOCK;
		clear_quest_bit = CLEAR_QUEST_BIT & INT_CONSTANT;
		do_event_chain = DO_EVENT_CHAIN & INT_CONSTANT;
		fade_from_black = FADE_FROM_BLACK;
		fade_from_white = FADE_FROM_WHITE;
		fade_to_black = FADE_TO_BLACK;
		fade_to_white = FADE_TO_WHITE;
		fill_screen = FILL_SCREEN & INT_CONSTANT;
		fill_screen_0 = FILL_SCREEN_0;
		load_pal = LOAD_PAL & INT_CONSTANT;
		npc_jump = NPC_JUMP & INT_CONSTANT & INT_CONSTANT & INT_CONSTANT;
		npc_lock = NPC_LOCK & INT_CONSTANT;
		npc_move = NPC_MOVE & INT_CONSTANT & INT_CONSTANT & INT_CONSTANT;
		npc_off = NPC_OFF & INT_CONSTANT;
		npc_on = NPC_ON & INT_CONSTANT;
		npc_text = NPC_TEXT & INT_CONSTANT & INT_CONSTANT;
		npc_turn = NPC_TURN & INT_CONSTANT & INT_CONSTANT;
		npc_unlock = NPC_UNLOCK & INT_CONSTANT;
		party_jump = PARTY_JUMP & INT_CONSTANT & INT_CONSTANT;
		party_member_text = PARTY_MEMBER_TEXT & INT_CONSTANT & INT_CONSTANT;
		party_move = PARTY_MOVE & INT_CONSTANT & INT_CONSTANT;
		party_off = PARTY_OFF;
		party_on = PARTY_ON;
		party_turn = PARTY_TURN & INT_CONSTANT;
		pause = PAUSE & INT_CONSTANT;
		play = PLAY & INT_CONSTANT;
		play_anim = PLAY_ANIM & INT_CONSTANT & INT_CONSTANT & INT_CONSTANT & INT_CONSTANT & INT_CONSTANT;
		show_map = SHOW_MAP;
		show_pic = SHOW_PIC & INT_CONSTANT & ~(INT_CONSTANT & INT_CONSTANT);
		show_picture = SHOW_PICTURE & INT_CONSTANT & INT_CONSTANT & INT_CONSTANT;
		song = SONG & INT_CONSTANT;
		sound = SOUND & INT_CONSTANT & INT_CONSTANT & INT_CONSTANT & INT_CONSTANT & INT_CONSTANT;
		sound_effect = SOUND_EFFECT & INT_CONSTANT & INT_CONSTANT & INT_CONSTANT & INT_CONSTANT & INT_CONSTANT;
		sound_fx_off = SOUND_FX_OFF;
		start_anim = START_ANIM & INT_CONSTANT & INT_CONSTANT & INT_CONSTANT & INT_CONSTANT;
		stop_anim = STOP_ANIM;
		text = TEXT & INT_CONSTANT;
		update = UPDATE & INT_CONSTANT;
		
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
		
		// NOTE: instead of actually constructing an AST, all script commands will be directly executed during parsing
		// matchers / script command execution:
		// TODO: all of them
		ambient.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		camera_jump.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		camera_lock.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		camera_move.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		camera_unlock.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		clear_quest_bit.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		do_event_chain.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		fade_from_black.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		fade_from_white.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		fade_to_black.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		fade_to_white.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		fill_screen.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		fill_screen_0.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		load_pal.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		npc_jump.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		npc_lock.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		npc_move.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		npc_off.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		npc_on.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		npc_text.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		npc_turn.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		npc_unlock.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		party_jump.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		party_member_text.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		party_move.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		party_off.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		party_on.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		party_turn.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		pause.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		play.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		play_anim.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		show_map.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		show_pic.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		show_picture.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		song.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		sound.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		sound_effect.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		sound_fx_off.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		start_anim.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		stop_anim.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		text.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		update.on_match([](auto& matches) -> parser_context::match_list {
			return {};
		});
		
#pragma clang diagnostic pop
	}
	
	unique_ptr<ast_node_base> parse(parser_context& ctx) const {
		auto tu_match = translation_unit.match(ctx);
		return (tu_match.matches.size() == 0 ? // match size might be 0 if this is an empty file or in error cases
				nullptr : move(tu_match.matches[0].ast_node));
	}
	
} albion_script_parser;

//
albion_script::albion_script() :
script_xlds({{ lazy_xld("SCRIPT0.XLD"), lazy_xld("SCRIPT2.XLD") }}) {
}

void albion_script::execute_script(const uint32_t& index_) {
	const size_t xld_index = (index_ < 100 ? 0 : 1);
	const size_t index = (index_ < 100 ? index_ : index_ - 200);
	
	auto script_data = script_xlds[xld_index].get_object_data(index);
	if(script_data == nullptr) {
		log_error("invalid script index %u!", index);
		return;
	}
	log_msg("script size for %u: %u", index, script_data->size());
	
	auto tu = make_unique<translation_unit>("script_" + to_string(index));
	tu->source.insert(0, (const char*)&(*script_data)[0], script_data->size());
	
	albion_script_lexer::map_characters(*tu);
	albion_script_lexer::lex(*tu);
	//albion_script_lexer::print_tokens(*tu);
	
	parser_context parser_ctx { *tu };
	//tu->ast = move(albion_script_parser.parse(parser_ctx));
	albion_script_parser.parse(parser_ctx);
	
	// if the end hasn't been reached, we have an error
	if(parser_ctx.iter != parser_ctx.end) {
		string error_msg = "parsing failed: ";
		if(parser_ctx.deepest_iter == tu->tokens.cend()) {
			error_msg += "premature EOF after";
			parser_ctx.deepest_iter = parser_ctx.end - 1; // set iter to token before EOF
		}
		else {
			error_msg += "possibly at";
		}
		error_msg += " \"" + parser_ctx.deepest_iter->second.to_string() + "\"";
		
		const auto line_and_column = albion_script_lexer::get_line_and_column_from_iter(*tu, parser_ctx.deepest_iter->second.begin);
		log_error("%s:%u:%u: %s",
				  tu->file_name, line_and_column.first, line_and_column.second, error_msg);
		//tu->ast.reset();
	}
}
