mergeInto(LibraryManager.library, {
    js_saveToLocalStorage: function(key, value) {
        var keyStr = UTF8ToString(key);
        var valueStr = UTF8ToString(value);
        localStorage.setItem(keyStr, valueStr);
    },
    js_loadFromLocalStorage: function(key) {
        var keyStr = UTF8ToString(key);
        var value = localStorage.getItem(keyStr);
        if (value === null) return 0;
        var bufferSize = lengthBytesUTF8(value) + 1;
        var buffer = _malloc(bufferSize);
        stringToUTF8(value, buffer, bufferSize);
        return buffer;
    }
});
