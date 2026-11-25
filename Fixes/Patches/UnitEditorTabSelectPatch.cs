using System;
using System.Linq;
using System.Reflection;
using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;
using AolLoader.Core;

namespace AolFixes.Patches
{
    /// <summary>
    /// Tab and Shift-Tab into a number field select its value, as a click does.
    ///
    /// The Transform panel's fields, and every other number field with units, are UnitEditor
    /// controls. A UnitEditor shows its value as text until it is edited; editing swaps in
    /// PART_TextEditor, a Serif TextEditor whose OnGotKeyboardFocus calls SelectAll. A click
    /// starts editing, so the value comes up selected. Tab only moves keyboard focus to the
    /// UnitEditor itself, which is a tab stop in display mode: it gets a focus ring, editing
    /// never starts, and nothing is selected. Typing still replaces the value, since a key
    /// press starts editing, but nothing on screen says so.
    ///
    /// When a UnitEditor gets keyboard focus from the keyboard, this starts editing, and
    /// Affinity's own code then focuses the editor and selects the value.
    ///
    /// Shift-Tab out of a field's editor lands on that same UnitEditor, because the editor is
    /// its child tab stop. Starting editing there again would trap Shift-Tab in the field, so
    /// focus that comes back to the field it just left, with Shift held, moves on to the
    /// previous field. That move is posted to the dispatcher: made inside the focus change, it
    /// bounced straight back.
    /// </summary>
    public static class UnitEditorTabSelectPatch
    {
        private static Type _unitEditorType;
        private static PropertyInfo _isTextEditing;
        private static PropertyInfo _isReadOnly;

        // The field a Tab key press left from, and whether Shift was held.
        private static DependencyObject _tabFrom;
        private static bool _tabShift;

        public static void Apply()
        {
            var serifAssembly = AppDomain.CurrentDomain.GetAssemblies()
                .FirstOrDefault(a => a.GetName().Name == "Serif.Affinity");
            _unitEditorType = serifAssembly?.GetType("Serif.Affinity.UI.Units.UnitEditor");
            _isTextEditing = _unitEditorType?.GetProperty("IsTextEditing");
            _isReadOnly = _unitEditorType?.GetProperty("IsReadOnly");

            if (_unitEditorType == null || _isTextEditing == null || _isTextEditing.PropertyType != typeof(bool))
            {
                Logger.Error("UnitEditor.IsTextEditing not found; Tab into number fields left as it is");
                return;
            }

            var dispatcher = Application.Current?.Dispatcher;
            if (dispatcher == null)
            {
                Logger.Error("No WPF application; Tab into number fields left as it is");
                return;
            }

            dispatcher.BeginInvoke(new Action(() =>
            {
                EventManager.RegisterClassHandler(_unitEditorType, Keyboard.GotKeyboardFocusEvent,
                    new KeyboardFocusChangedEventHandler(UnitEditor_GotKeyboardFocus), true);
                EventManager.RegisterClassHandler(typeof(TextBoxBase), Keyboard.PreviewKeyDownEvent,
                    new KeyEventHandler(TextBox_PreviewKeyDown), true);
                Logger.Info("Registered the Tab-into-number-field handlers");
            }));
        }

        private static void TextBox_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key != Key.Tab)
                return;
            var d = sender as DependencyObject;
            while (d != null && !_unitEditorType.IsInstanceOfType(d))
                d = VisualTreeHelper.GetParent(d);
            _tabFrom = d;
            _tabShift = (Keyboard.Modifiers & ModifierKeys.Shift) != 0;
        }

        private static void UnitEditor_GotKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            try
            {
                if (!ReferenceEquals(sender, e.OriginalSource))
                    return;
                if (!(InputManager.Current.MostRecentInputDevice is KeyboardDevice))
                    return;
                if ((bool)_isTextEditing.GetValue(sender))
                    return;
                if (_isReadOnly != null && _isReadOnly.PropertyType == typeof(bool) && (bool)_isReadOnly.GetValue(sender))
                    return;

                if (ReferenceEquals(sender, _tabFrom) && _tabShift)
                {
                    _tabFrom = null;
                    var element = (UIElement)sender;
                    element.Dispatcher.BeginInvoke(DispatcherPriority.Input, new Action(() =>
                        element.MoveFocus(new TraversalRequest(FocusNavigationDirection.Previous))));
                    return;
                }

                _tabFrom = null;
                _isTextEditing.SetValue(sender, true);
            }
            catch (Exception ex)
            {
                Logger.Error("Tab into number field", ex);
            }
        }
    }
}
