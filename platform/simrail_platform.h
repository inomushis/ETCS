 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "platform.h"

class SimrailBasePlatform final : public BasePlatform {
private:
	class SimrailBusSocket final : public BusSocket {
	private:
		uint32_t handle;
	public:
		SimrailBusSocket(uint32_t handle);
		virtual ~SimrailBusSocket() override;
		virtual void broadcast(const std::string &data) override;
		virtual void broadcast(uint32_t tid, const std::string &data) override;
		virtual void send_to(uint32_t uid, const std::string &data) override;
		virtual PlatformUtil::Promise<std::pair<PeerId, std::string>> receive() override;
		virtual PlatformUtil::Promise<PeerId> on_peer_join() override;
		virtual PlatformUtil::Promise<PeerId> on_peer_leave() override;
	};

public:
	SimrailBasePlatform();

	virtual int64_t get_timer() override;
	virtual int64_t get_timestamp() override;
	virtual DateTime get_local_time() override;

	virtual std::unique_ptr<BusSocket> open_socket(const std::string &channel, uint32_t tid) override;
	virtual std::string read_file(const std::string &path) override;
	virtual void write_file(const std::string &path, const std::string &contents) override;
	virtual void debug_print(const std::string &msg) override;

	virtual PlatformUtil::Promise<void> delay(int ms) override;
	virtual PlatformUtil::Promise<void> on_quit_request() override;
	virtual PlatformUtil::Promise<void> on_quit() override;

	virtual void quit() override;
};

class SimrailUiPlatform final : public UiPlatform {
private:
	SimrailBasePlatform base;

public:
	class SimrailImage : public Image
	{
	public:
		SimrailImage() = default;
		virtual float width() const override { return 0.0f; };
		virtual float height() const override { return 0.0f; };
	};

	class SimrailFont : public Font
	{
	public:
		SimrailFont() = default;
		virtual float ascent() const override { return 0.0f; }
		virtual std::pair<float, float> calc_size(const std::string &str) const override { return std::make_pair(0.0f, 0.0f); }
	};

	class SimrailSoundData : public SoundData
	{
	public:
		SimrailSoundData() = default;
	};

	class SimrailSoundSource : public SoundSource
	{
	public:
		SimrailSoundSource() = default;
		virtual void detach() override {};
	};

	virtual int64_t get_timer() override { return base.get_timer(); };
	virtual int64_t get_timestamp() override { return base.get_timestamp(); };
	virtual DateTime get_local_time() override { return base.get_local_time(); };

	virtual std::unique_ptr<BusSocket> open_socket(const std::string &channel, uint32_t tid) override { return base.open_socket(channel, tid); };
	virtual std::string read_file(const std::string &path) override { return base.read_file(path); };
	virtual void write_file(const std::string &path, const std::string &contents) override { base.write_file(path, contents); };
	virtual void debug_print(const std::string &msg) override { base.debug_print(msg); };

	virtual PlatformUtil::Promise<void> delay(int ms) override { return base.delay(ms); };
	virtual PlatformUtil::Promise<void> on_quit_request() override { return base.on_quit_request(); };
	virtual PlatformUtil::Promise<void> on_quit() override { return base.on_quit(); };

	virtual void quit() override { base.quit(); };

	virtual void set_color(Color c) override;
	virtual void draw_line(float x1, float y1, float x2, float y2) override;
	virtual void draw_rect(float x, float y, float w, float h) override;
	virtual void draw_rect_filled(float x, float y, float w, float h) override;
	virtual void draw_image(const Image &img, float x, float y, float w, float h) override;
	virtual void draw_circle_filled(float x, float y, float rad) override;
	virtual void draw_polygon_filled(const std::vector<std::pair<float, float>> &poly) override;
	virtual void clear() override;
	virtual PlatformUtil::Promise<void> present() override;
	virtual std::unique_ptr<Image> load_image(const std::string &path) override;
	virtual std::unique_ptr<Font> load_font(float size, bool bold) override;
	virtual std::unique_ptr<Image> make_text_image(const std::string &text, const Font &font, Color c) override;
	virtual std::unique_ptr<Image> make_wrapped_text_image(const std::string &text, const Font &font, int align, Color c) override;

	virtual void set_volume(int vol) override;
	virtual std::unique_ptr<SoundData> load_sound(const std::string &path) override;
	virtual std::unique_ptr<SoundData> load_sound(const std::vector<std::pair<int, int>> &melody) override;
	virtual std::unique_ptr<SoundSource> play_sound(const SoundData &snd, bool looping) override;

	virtual void set_brightness(int br) override;

	virtual PlatformUtil::Promise<InputEvent> on_input_event() override;
};
