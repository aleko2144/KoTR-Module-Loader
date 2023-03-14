# KoTR-Module-Loader
Загрузчик b3d-файлов для игры "Дальнобойщики - II" версии 8.2 / King of The Road *.b3d loader

Файлы "detours.h", "detours.lib", "detver.h", "sigscan.h" [взяты отсюда](https://github.com/Zer0Mem0ry/Detour/tree/master/dll).
Огромная благодарность Voron295 и [Duude92](https://github.com/Duude92).

### Использование
ModuleLoader.ini:
```
[FILE_<индекс>]
path = путь к модулю (с двумя обратными косыми, например, "vehicles\\")
name = имя модуля (например, "Fiat")
```
Индекс начинается с единицы.

### Особенности
Игра может вылетать при загрузке мира. Вероятно, проблема в работе функции
GetFullPath. В таком случае обычно появляется сообщение в файле warn.log с
описанием ошибки (неправильный путь к модулю).

### Ссылки
[VK](https://vk.com/rnr_mods), [Discord](https://discord.gg/cTmC8VhUGp)
