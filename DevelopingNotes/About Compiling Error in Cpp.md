# 让人崩溃的编译错误

本来已经可以运行的程序代码，由于添加了一行代码，用于增加一个arcball球体的绘制。结果出现了一个小错误，调试中于是使用了xcode自身的fix功能修复，结果莫名其妙的在某个头文件的结尾处添加了两个"\*\*"。然后悲剧就出现了，整个项目出现了20多处编译错误，大多是C++标准库的错误。

措手不及之后，开始调试，注意到大部分是标准库的错误，于是认为是xcode项目的设置（比如编译器等building settings）被改动，就拿着正常的项目在数也数不清的配置项中比对，比对不出结果。又从网上搜寻方案，在一通通的改动之后，错误数目不停的减少，最后减少为一个ImageTexture错误（`return type out-of-line definition differs from that in the decalaration`)。然而这肯定也是徒劳，就如开头所说，真正的错误原因不在此处。

最后做了最不想做的事，就是备份代码，然后将项目从git上恢复到上一次checkin时的状态，随后将一个个文件加进去，最初从main文件开始，一个个文件确认，最后确认只是material.h文件的末尾被xcode的fix功能莫名奇妙的加入了两个“\*\*”，由此而引出的抓耳挠腮式的调试过程。

图示所呈现出的是南辕北辙式的错误信息，CPP编译器对非正常错误的自作多情是如此严重。
![Figure1](media/2019-12-11-figure1.png)

![Figure2](media/2019-12-11-figure2.png)

