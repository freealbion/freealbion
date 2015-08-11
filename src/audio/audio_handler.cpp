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

#include "audio_handler.hpp"
#include <floor/audio/audio_controller.hpp>

//#define AR_AUDIO_HANDLER_DEBUG_UI 1
#if defined(AR_AUDIO_HANDLER_DEBUG_UI)
static gui_window* ah_wnd { nullptr };
static gui_list_box* ah_sample_list { nullptr };
#endif

array<lazy_xld, 3> audio_handler::sample_xlds;
lazy_xld audio_handler::wavelib_xld;

void audio_handler::init() {
	sample_xlds = array<lazy_xld, 3> {{ lazy_xld("SAMPLES0.XLD"), lazy_xld("SAMPLES1.XLD"), lazy_xld("SAMPLES2.XLD") }};
	wavelib_xld = lazy_xld { "WAVELIB0.XLD" };
	
#if defined(AR_AUDIO_HANDLER_DEBUG_UI)
	ah_wnd = ui->add<gui_window>(float2(0.3f, 1.0f), float2(0.0f, 0.0f));
	static constexpr float margin_x = 0.025f, margin_x_double = margin_x * 2.0f;
	static constexpr float margin_y = 0.0125f, margin_y_double = margin_y * 2.0f;
	static constexpr float x_full_width = 1.0f - margin_x_double;
	const float list_height = 1.0f - margin_y_double;
	ah_sample_list = ui->add<gui_list_box>(float2(x_full_width, list_height), float2(margin_x, margin_y));
	ah_wnd->add_child(ah_sample_list);
	
	ah_sample_list->add_handler([](GUI_EVENT, gui_object&) {
		const auto selected_item = ah_sample_list->get_selected_item();
		if(selected_item == nullptr) return;
		
		floor::acquire_context();
		if(selected_item->first[0] == 's') {
			audio_handler::play_sample(string2size_t(selected_item->first.substr(1)));
		}
		else if(selected_item->first[0] == 'w') {
			const auto comma_pos = selected_item->first.find(',');
			const auto lib_idx = string2size_t(selected_item->first.substr(1, comma_pos - 1));
			const auto sample_idx = string2size_t(selected_item->first.substr(comma_pos + 1));
			audio_handler::play_wavelib_sample(lib_idx, sample_idx);
		}
		floor::release_context();
	}, GUI_EVENT::LIST_BOX_SELECT_EXECUTE);
	
	const auto bytes_to_ms = [](const size_t& size) {
		const auto freq = 11050u;
		const auto depth = 1u; // 8-bit
		const auto channels = 1u;
		return (size * 1000u) / (freq * depth * channels);
	};
	for(size_t i = 0; i < wavelib_xld.get_object_count(); ++i) {
		const auto lib_size = wavelib_xld.get_object_size(i);
		if(lib_size == 0) continue;
		
		auto lib_data = wavelib_xld.get_object_data(i);
		size_t data_size = wavelib_max_entries * wavelib_entry_size;
		for(size_t j = 0; j < wavelib_max_entries; ++j) {
			const wavelib_entry* entry_header = (const wavelib_entry*)&(*lib_data)[wavelib_entry_size * j];
			if(entry_header->index == 0xFFFFFFFF) continue;
			
			ah_sample_list->add_item("w" + to_string(i) + "," + to_string(j),
									 "Wavelib #" + to_string(i) + "/" + to_string(j) + " (" + to_string(entry_header->length) + " bytes / " +
									 to_string(bytes_to_ms(entry_header->length)) + "ms / " + to_string(entry_header->frequency) + "Hz)");
			data_size += entry_header->length;
		}
		if(data_size != lib_size) {
			cout << ">> wavelib: unused data in wavelib #" << i << endl;
		}
	}
	for(size_t i = 0; i < sample_xlds.size(); ++i) {
		for(size_t j = 0; j < sample_xlds[i].get_object_count(); ++j) {
			const auto sample_size = sample_xlds[i].get_object_size(j);
			if(sample_size == 0) continue;
			const auto sample_idx = to_string((i * 100) + j);
			ah_sample_list->add_item("s" + sample_idx,
									 "Sample #" + sample_idx + " (" + to_string(sample_size) + " bytes / " +
									 to_string(bytes_to_ms(sample_size)) + "ms)");
		}
	}
#endif
}

void audio_handler::destroy() {
}

void audio_handler::play_sample(const size_t& index) {
	if(index >= 300) return;
	
	auto sample_data = sample_xlds[index / 100].get_object_data(index % 100);
	if(!sample_data || sample_data->size() == 0) return;
	
	auto store_ptr = audio_store::add_raw(&(*sample_data)[0], (ALsizei)sample_data->size(), AL_FORMAT_MONO8, 11050, "SAMPLE_" + to_string(index));
	auto sample_src_ptr = audio_controller::add_source(store_ptr);
	auto sample_src = sample_src_ptr.lock();
	sample_src->play();
}

void audio_handler::play_wavelib_sample(const size_t& wavelib_index, const size_t& sample_idx) {
	// each wavelib contains 512 entries max
	if(sample_idx >= wavelib_max_entries) return;
	
	auto wavelib = wavelib_xld.get_object_data(wavelib_index);
	if(!wavelib || wavelib->size() == 0) return;
	
	const wavelib_entry* sample_header = (const wavelib_entry*)&(*wavelib)[wavelib_entry_size * sample_idx];
	
	/*cout << "wavelib sample: " << sample_header->index << ", " << sample_header->start_offset << ", ";
	cout << sample_header->length << ", " << sample_header->frequency;
	cout << " /// unknown: " << sample_header->_unknown_0 << ", " << sample_header->_unknown_1 << ", " << sample_header->_unknown_2 << ", " << sample_header->_unknown_3 << endl;*/
	
	/*for(size_t i = 0; i < wavelib_max_entries; ++i) {
		const wavelib_entry* sample_header = (const wavelib_entry*)&(*wavelib)[wavelib_entry_size * i];
		cout << "wavelib sample: idx: " << sample_header->index << ", offset: " << sample_header->offset << ", len: ";
		cout << sample_header->length << ", freq: " << sample_header->frequency;
		cout << " /// unknown: " << sample_header->_unknown_0 << ", " << sample_header->_unknown_1 << ", " << sample_header->_unknown_2 << ", " << sample_header->_unknown_3 << endl;
	}*/
	
	auto store_ptr = audio_store::add_raw(&(*wavelib)[sample_header->offset], (ALsizei)sample_header->length, AL_FORMAT_MONO8,
										  (ALsizei)sample_header->frequency, "WAVELIB_" + to_string(wavelib_index) + "_" + to_string(sample_idx));
	auto sample_src_ptr = audio_controller::add_source(store_ptr);
	auto sample_src = sample_src_ptr.lock();
	sample_src->play();
}
