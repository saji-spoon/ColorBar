
# include <Siv3D.hpp>
#include"ColorBar.hpp"


void Main()
{
	const Font font(30);

	ColorBar cb;

	Graphics::SetBackground(Color(60,60,60));

	while (System::Update())
	{
		cb.draw();

		cb.update();
	}
}
