#pragma once
#include<Siv3D.hpp>

//m_basePosからx方向にm_maxまでの間でドラッグして動かせる
//get()で現在位置を取得できる（0.0～1.0）
class Slider
{
public:
	Slider()
	{
	}

	Slider(const Point& pos, const int max)
		:m_basePos(pos),
		m_max(max),
		m_x(0),
		m_state(Normal)
	{
	}

	enum State
	{
		Normal,
		Dragged
	};

	Point m_basePos;

	int m_max;

	int m_x;

	//ドラッグされているかどうかの状態をもつ
	State m_state;

	void draw()const
	{
		Rect(10, 50).setCenter(m_basePos + Point(m_x, 0)).draw(Palette::White);
	}

	void update()
	{
		switch (m_state)
		{
		case Slider::Normal:
			break;
		case Slider::Dragged:
			m_x = Clamp(m_x + Mouse::Delta().x, 0, m_max);
			break;
		default:
			break;
		}

		const Rect body = Rect(10, 50).setCenter(m_basePos + Point(m_x, 0));

		if (m_state == Normal && body.leftPressed)
		{
			m_state = Dragged;
		}
		else if (m_state == Dragged && !(Input::MouseL.pressed))
		{
			m_state = Normal;
		}

	}

	//現在位置を0.0（左端）～1.0（右端）で取得
	double get()const
	{
		return 1.0 * m_x / m_max;
	}
};

//スライダー式カラーバー
//ドラッグしてスライダーを動かして色を選択できる
//プレビュー用に選択中の色の■とRGB値が表示される
class ColorBar
{
public:
	//INIファイルから設定値を読み込んで格納しておく
	//・レイアウトの都合上、INIファイルの値をそのまま使うのではなく、1つの値を何回か使って計算することが多くなりそう
	//・更新されたINIファイルのロードやファイル読み込み時の管理がある
	//ことを考慮して別クラス化
	class Layout
	{
	public:
		Layout() {}

		Layout(const FilePath& INIFile) :
			m_INI(INIFile)
		{
			m_valid = true;

			if (!m_INI)
			{
				//INIファイルが読み込めなかった場合、以下の定数をセットし、その値でINIファイルを新しく作る
				m_pos = Point(100, 20);
				m_barSize = Rect(300, 40);
				m_xPadding = 40;
				m_fineness = 50;
				m_sliderMargin = 60;
				m_colSamplePos = Point(500, 40);
				m_colStrPos = Point(0, 20);

				INIWriter writer(L"defaultLayout.INI");
				writer.write(L"pos", m_pos);
				writer.write(L"barSize", m_barSize);
				writer.write(L"xPadding", m_xPadding);
				writer.write(L"fineness", m_fineness);
				writer.write(L"barMargin", m_sliderMargin);
				writer.write(L"colSamplePos", m_colSamplePos);
				writer.write(L"colStrPos", m_colStrPos);

				m_valid = false;

				LOG_ERROR(L"Layout:読み込みに失敗");

				return;
			}

			load();
		}

		INIReader m_INI;

		Rect m_barSize;
		Point m_pos;
		int m_xPadding;
		int m_yPadding = 30;
		int m_fineness;
		int m_sliderMargin;

		Point m_colSamplePos;
		Point m_colStrPos;

		bool m_valid;

		bool update()
		{
			const bool c = m_INI.hasChanged();

			if (c)
			{
				m_INI.reload();

				load();
			}

			return c;
		}

		//各種値を読み込む
		//値の意味はINIファイルを参照
		void load()
		{
			m_barSize = m_INI.getOr<Rect>(L"barSize", Rect(300, 40));
			m_pos = m_INI.getOr<Point>(L"pos", Point(100, 20));
			m_xPadding = m_INI.getOr<int>(L"xPadding", 40);
			m_fineness = m_INI.getOr<int>(L"fineness", 50);
			m_sliderMargin = m_INI.getOr<int>(L"barMargin", 60);
			m_colSamplePos = m_INI.getOr<Point>(L"colSamplePos", Point(500, 40));
			m_colStrPos = m_INI.getOr<Point>(L"colStrPos", Point(0, 40));
		}

		void setINI(FilePath& INIFile)
		{
			INIReader ini(INIFile);

			if (ini)
			{
				m_INI = std::move(ini);
			}
			else
			{
				LOG_ERROR(L"setINI:読み込みに失敗:" + INIFile);
			}
		}

	} m_layout;

private:
	//RGB値表示用フォント
	Font m_font;

	Slider m_sliderR;
	Slider m_sliderG;
	Slider m_sliderB;

	//選択中の色
	ColorF m_col;

public:
	ColorBar() :
		m_layout(L"defaultLayout.ini"),
		m_font(16),
		m_col(Palette::Black)
	{
		setSliderFromLayout();
	}

	void setColor(const ColorF& c)
	{
		m_col = c;
	}

	void draw()const
	{
		const int frameX = m_layout.m_pos.x + m_layout.m_colSamplePos.x + 120 + m_layout.m_xPadding;
		const int frameY = m_layout.m_pos.y + m_layout.m_yPadding +
			Max(m_layout.m_yPadding + 2 * (m_layout.m_barSize.h + m_layout.m_sliderMargin) + m_layout.m_barSize.h, m_layout.m_colSamplePos.y + 120);

		const Rect uiRect(m_layout.m_pos, Point(frameX, frameY) - m_layout.m_pos);

		uiRect.draw(Palette::Dimgray);
		uiRect.drawFrame(0.0, 3.0, Palette::Darkgray);

		m_sliderR.draw();
		m_sliderG.draw();
		m_sliderB.draw();

		//ColorBarのグラデーションは少しずつ色を変えた小さなRectを並べて描画している
		//SpectorWidth…小さなRectの横の長さ
		const int fineness = m_layout.m_fineness;

		const double SpectorWidth = 1.0 * m_layout.m_barSize.w / fineness;

		const double deltaColor = 1.0 / fineness;

		const Point pos = m_layout.m_pos;

		for (int i = 0; i < fineness; ++i)
		{
			RectF(pos + Vec2(m_layout.m_xPadding + SpectorWidth*i, m_layout.m_yPadding), Vec2(SpectorWidth, m_layout.m_barSize.h)).draw(ColorF(0.0 + i*deltaColor, m_col.g, m_col.b));
			RectF(pos + Vec2(m_layout.m_xPadding + SpectorWidth*i, m_layout.m_yPadding + m_layout.m_barSize.h + m_layout.m_sliderMargin), Vec2(SpectorWidth, m_layout.m_barSize.h)).draw(ColorF(m_col.r, 0.0 + i*deltaColor, m_col.b));
			RectF(pos + Vec2(m_layout.m_xPadding + SpectorWidth*i, m_layout.m_yPadding + 2 * (m_layout.m_barSize.h + m_layout.m_sliderMargin)), Vec2(SpectorWidth, m_layout.m_barSize.h)).draw(ColorF(m_col.r, m_col.g, 0.0 + i*deltaColor));
		}

		Rect(pos + m_layout.m_colSamplePos, 120).draw(m_col);

		const Color tempCol(m_col);

		m_font.draw(Format(L"R:", tempCol.r, L" G:", tempCol.g, L" B:", tempCol.b), pos + m_layout.m_colStrPos);


	}

	void update()
	{
		const bool iniChanged = m_layout.update();

		if (iniChanged)
		{
			setSliderFromLayout();
		}

		m_sliderR.update();
		m_sliderG.update();
		m_sliderB.update();

		m_col.r = m_sliderR.get();
		m_col.g = m_sliderG.get();
		m_col.b = m_sliderB.get();
	}

	void setSliderFromLayout()
	{
		m_sliderR = Slider(m_layout.m_pos + Point(m_layout.m_xPadding, m_layout.m_yPadding + m_layout.m_barSize.h / 2), m_layout.m_barSize.w);
		m_sliderG = Slider(m_layout.m_pos + Point(m_layout.m_xPadding, m_layout.m_yPadding + m_layout.m_barSize.h / 2 + m_layout.m_barSize.h + m_layout.m_sliderMargin), m_layout.m_barSize.w);
		m_sliderB = Slider(m_layout.m_pos + Point(m_layout.m_xPadding, m_layout.m_yPadding + m_layout.m_barSize.h / 2 + 2 * (m_layout.m_barSize.h + m_layout.m_sliderMargin)), m_layout.m_barSize.w);
	}

	void print()
	{

	}
};
