# MZZ Game Engine

<p>I'm currently developing this engine, the idea basicaly turn around a tile map and a spritesheet from where you create equal sized GameObjects and "plug" components to them (like animation, ai, input handlers, particles and more) . The games are articulated in Levels that are managed by an State object that also manage the state of the game (menu, pause, playing). There is also UI components like Menu and Buttons  </p>

# Examples
<p>Note: this is under active develpment so the examples are a little raw, the final idea is to abstract all the SDL2 parts.</p>

### Creating a GameObject
	//x,y,w,h,asset_tile_x,asset_tile_y,tag
	GameObject player{ 0,0,32,32,29,0, "player" };
	 

	
### Inherit the input handler component
	struct PlayerInputHandler : InputHandler
	{
		const Uint8* kbstate = SDL_GetKeyboardState(NULL);
		void handle_input(GameObject& gameobject) override {
			if (!kbstate[SDL_SCANCODE_RIGHT]) gameobject.x++;
		}
	};

### Pluging the component
	player.set_input_handler(std::make_unique<PlayerInputHandler>());

### Demo
In the demo folder I wil be uploading the examples that I use to test features

<img src="Mzzgj\Mzzgj\demo1.webp">