#pragma once
#include<Siv3D.hpp>

//m_basePos����x������m_max�܂ł̊ԂŃh���b�O���ē�������
//get()�Ō��݈ʒu���擾�ł���i0.0�`1.0�j
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

	//�h���b�O����Ă��邩�ǂ����̏�Ԃ�����
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

	//���݈ʒu��0.0�i���[�j�`1.0�i�E�[�j�Ŏ擾
	double get()const
	{
		return 1.0 * m_x / m_max;
	}
};

//�X���C�_�[���J���[�o�[
//�h���b�O���ăX���C�_�[�𓮂����ĐF��I���ł���
//�v���r���[�p�ɑI�𒆂̐F�́���RGB�l���\�������
class ColorBar
{
public:
	//INI�t�@�C������ݒ�l��ǂݍ���Ŋi�[���Ă���
	//�E���C�A�E�g�̓s����AINI�t�@�C���̒l�����̂܂܎g���̂ł͂Ȃ��A1�̒l�����񂩎g���Čv�Z���邱�Ƃ������Ȃ肻��
	//�E�X�V���ꂽINI�t�@�C���̃��[�h��t�@�C���ǂݍ��ݎ��̊Ǘ�������
	//���Ƃ��l�����ĕʃN���X��
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
				//INI�t�@�C�����ǂݍ��߂Ȃ������ꍇ�A�ȉ��̒萔���Z�b�g���A���̒l��INI�t�@�C����V�������
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

				LOG_ERROR(L"Layout:�ǂݍ��݂Ɏ��s");

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

		//�e��l��ǂݍ���
		//�l�̈Ӗ���INI�t�@�C�����Q��
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
				LOG_ERROR(L"setINI:�ǂݍ��݂Ɏ��s:" + INIFile);
			}
		}

	} m_layout;

private:
	//RGB�l�\���p�t�H���g
	Font m_font;

	Slider m_sliderR;
	Slider m_sliderG;
	Slider m_sliderB;

	//�I�𒆂̐F
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

		//ColorBar�̃O���f�[�V�����͏������F��ς���������Rect����ׂĕ`�悵�Ă���
		//SpectorWidth�c������Rect�̉��̒���
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
