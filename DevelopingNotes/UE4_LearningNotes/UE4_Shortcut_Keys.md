# UE4视口（viewport）操作快捷键整理

在UE4编辑器中，当在视口中操纵actors，如果单独使用鼠标移动，经常会感觉一些精细操作比较困难。实际上，结合UE4提供的快捷键，再配合鼠标操作，进行精细操作其实是可行的。下面是视口操作中所需要的快捷键操作整理。

## 1. 视口基础操作（Viewport Basics）

    1. 使用ALT键+G/H/J/K键，在透视视口(perspective viewport)/前视口(front viewport)/顶视口(top viewport)/左视口(left viewport)切换。后视口(back viewport)/底视口(bottom viewport)/右视口(right viewport)则分别在前/顶/左视口的快捷键基础上再加上SHIFT键。
    2. G键，可以启用游戏视图（隐藏不可见actor和辅助工具）
    3. F11键，切换到沉浸式视口（将当前视口全屏化）

## 2. Viewport Controls - 鼠标

    1. 鼠标左键(LMB)+拖动，前后（左右）移动相机
    2. 鼠标右键(RMB)+拖动，旋转相机
    3. 鼠标左右键(LMB+RMB)+拖动，上下移动相机
    4. F键，将选定actor快速聚焦到屏幕中心

## 3. 视口控制操作 - 平移、环绕和缩放（maya风格）

    1. ALT键+鼠标左键（LMB）+拖动，以鼠标点击处为支点进行左右和上下视口转动
    2. ALT键+鼠标右键（RMB）+拖动，前后推动相机
    3. ALT键+鼠标中键（MMB）+拖动，使相机跟踪鼠标运动左右/上下移动

## 3. 视口控制操作 - 键盘（游戏风格）
以游戏风格操作视口的前提是摁住鼠标右键（左键也可以）

    1. W键，向前移动相机
    2. S键，向后移动相机
    3. A键，向左移动相机
    4. D键, 向右移动相机
    5. E键，向上移动相机
    6. Q键，向下移动相机
    7. Z键, 增加FOV（缩小图像）
    8. C键，减小FOV（放大图像）

注：增加FOV或缩小FOV时，当松开鼠标右键后，视口会闪回到原来的状态。

## 4. 视口控制操作 - 变换工具
变换工具用于在视口中移动、旋转、缩放演员（actor），在视口中，选中actor后，可以使用下面的键位：

    1. W键，选择移动工具（move tool）
    2. E键，选择旋转工具（rotate tool）
    3. R键，选择缩放工具（scale tool）

移动工具的特殊控制：
    
    1. ALT+鼠标左键（LMB）+拖动（需要鼠标点击在move tool的方向上），生成当前actor的副本，并可以移动，但是不影响原始的actor。

透视视口下的移动工具控制：

    1. CTRL+鼠标左键（LMB）+拖动，沿着x轴移动actor
    2. CTRL+鼠标右键（RMB）+拖动，沿着y轴移动actor
    3. CTRL+鼠标左右键（LMB+RMB）+拖动，沿着z轴移动actor

透视视口下的旋转工具控制：

    1. CTRL+鼠标左键（LMB）+拖动，沿着x轴旋转actor
    2. CTRL+鼠标右键（RMB）+拖动，沿着y轴旋转actor
    3. CTRL+鼠标左右键（LMB+RMB）+拖动，沿着z轴旋转actor

缩放工具控制（正交和透视视口都适用）：

    1. CTRL+鼠标左键（LMB）+拖动，沿着所有轴一致缩放actor

正交视口下的移动工具控制：

    1. CTRL+鼠标左键（LMB）+拖动，沿着当前选定平面的两个轴移动actor
    
正交视口下的缩放工具控制：

    1. CTRL+鼠标左键（LMB）+拖动，沿着可视轴旋转actor