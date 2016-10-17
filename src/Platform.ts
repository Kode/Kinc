export interface PC {
	Windows: string;
	WindowsApp: string;
	PlayStation3: string;
	iOS: string;
	OSX: string;
	Android: string;
	Xbox360: string;
	Linux: string;
	HTML5: string;
	Tizen: string;
	Pi: string;
	tvOS: string;
	[key: string]: string;
};

export let Platform: PC = {
	Windows: 'windows',
	WindowsApp: 'windowsapp',
	PlayStation3: 'ps3',
	iOS: 'ios',
	OSX: 'osx',
	Android: 'android',
	Xbox360: 'xbox360',
	Linux: 'linux',
	HTML5: 'html5',
	Tizen: 'tizen',
	Pi: 'pi',
	tvOS: 'tvos'
};
